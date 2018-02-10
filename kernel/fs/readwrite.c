#include <fs/fs.h>
#include <fs/devfile.h>
#include <mm/memset.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <sched/spinlock.h>
#include <errno.h>
#include <sched/thread.h> //for jiffyCounter
#include <print.h>

struct CfEntry { //describes 1 page of file data
	void *addr;
	uint64_t fileOffset;
	bool dirty;
	uint32_t lastAccessed;
};

struct CachedFile {
	unsigned int nrofEntries;
	struct CfEntry entries[1];
};

ssize_t fsRead(struct File *file, void *buffer, size_t bufSize) {
	acquireSpinlock(&file->lock);
	struct Inode *inode = file->inode;
	acquireSpinlock(&inode->lock);

	ssize_t bytesCopied = 0;
	if ((inode->type & ITYPE_MASK) == ITYPE_DIR) {
		bytesCopied = -EISDIR;
		goto ret;
	}

	if ((inode->type & ITYPE_MASK) == ITYPE_CHAR) {
		bytesCopied = -ENOSYS;
		struct DevFileOps *ops = inode->ops;
		if (ops && ops->read) {
			bytesCopied = ops->read(file, buffer, bufSize);
		}
		goto ret;
	}

	size_t bytesLeft = inode->fileSize - file->offset;
	if (bytesLeft > bufSize) {
		bytesLeft = bufSize;
	}
	
	if (inode->ramfs & RAMFS_INITRD) {
		memcpy(buffer, inode->cachedData + 1 + file->offset, bytesLeft);
		file->offset += bytesLeft;

		bytesCopied = bytesLeft;
		goto ret;
	}

	struct CachedFile *cf = inode->cachedData;

	while (bytesLeft) {
		struct CfEntry *entry = NULL;
		uint64_t offset;
		for (unsigned int i = 0; i < cf->nrofEntries; i++) {
			offset = cf->entries[i].fileOffset;
			if (file->offset >= offset && file->offset < offset + PAGE_SIZE) {
				entry = &cf->entries[i];
				break;
			}
		}
		if (!entry) {
			//load entry from fs, reacquire spinlocks and reload cf
			printk("Error reading file: unimplemented");
			bytesCopied = -ENOSYS;
			goto ret;
			//continue;
		}
		entry->lastAccessed = jiffyCounter;
		unsigned int diff = file->offset - offset;
		//unsigned int nrofBytes = PAGE_SIZE - diff;
		unsigned int nrofBytes = ((bytesLeft > PAGE_SIZE)? PAGE_SIZE : bytesLeft) - diff;
		memcpy(buffer, (void *)((uintptr_t)(entry->addr) + diff), nrofBytes);

		buffer = (void *)((uintptr_t)buffer + nrofBytes);
		file->offset += nrofBytes;
		bytesLeft -= nrofBytes;
		bytesCopied += nrofBytes;
	}

	ret:
	releaseSpinlock(&inode->lock);
	releaseSpinlock(&file->lock);
	return bytesCopied;
}

int fsWrite(struct File *file, void *buffer, size_t bufSize) {
	acquireSpinlock(&file->lock);
	struct Inode *inode = file->inode;
	acquireSpinlock(&inode->lock);

	int error = 0;
	if ((inode->type & ITYPE_MASK) == ITYPE_DIR) {
		error = -EISDIR;
		goto ret;
	}

	if ((inode->type & ITYPE_MASK) == ITYPE_CHAR) {
		error = -ENOSYS;
		struct DevFileOps *ops = inode->ops;
		if (ops && ops->write) {
			error = ops->write(file, buffer, bufSize);
		}
		goto ret;
	}

	if (inode->ramfs & RAMFS_INITRD) {
		error = -EROFS;
		goto ret;
	}

	struct CachedFile *cf = inode->cachedData;
	
	while (bufSize) {
		inode->cacheDirty = true;
		if (file->offset == inode->fileSize && !(inode->fileSize % PAGE_SIZE)) { //offset can never be greater
			unsigned int nrofPages = bufSize / PAGE_SIZE;
			if (bufSize % PAGE_SIZE) {
				nrofPages++;
			}
			
			size_t newCFSize;
			if (file->offset) {
				//create new cf entries
				newCFSize = inode->cachedDataSize + (sizeof(struct CfEntry) * nrofPages);
				struct CachedFile *newCF = krealloc(cf, newCFSize);
				if (!newCF) {
					error = -ENOMEM;
					goto ret;
				}
				cf = newCF;
			} else {
				//create new cf
				newCFSize = sizeof(struct CachedFile) + (nrofPages - 1) * sizeof(struct CfEntry);
				cf = kmalloc(newCFSize);
				if (!cf) {
					error = -ENOMEM;
					goto ret;
				}
				cf->nrofEntries = 0;
			}
			inode->cachedDataSize = newCFSize;
			inode->cachedData = cf;

			for (unsigned int i = 0; i < nrofPages; i++) {
				//alloc page
				void *page = allocKPages(PAGE_SIZE, PAGE_FLAG_WRITE);
				if (!page) {
					error = -ENOMEM;
					goto ret;
				}

				//map it in the entry
				cf->entries[cf->nrofEntries].addr = page;
				cf->entries[cf->nrofEntries].fileOffset = file->offset + i * PAGE_SIZE;
				cf->entries[cf->nrofEntries].dirty = true;
				cf->entries[cf->nrofEntries].lastAccessed = jiffyCounter;

				cf->nrofEntries += 1;

				//and write
				unsigned int copy = (bufSize > PAGE_SIZE)? PAGE_SIZE : bufSize;
				memcpy(page, buffer, copy);
				buffer = (void *)((uintptr_t)buffer + copy);
				bufSize -= copy;
				file->offset += copy;
				inode->fileSize += copy;
			}
			
			break; //bufsize should be zero
		}

		//find entry
		struct CfEntry *entry = NULL;
		uint64_t offset;
		for (unsigned int i = 0; i < cf->nrofEntries; i++) {
			offset = cf->entries[i].fileOffset;
			if (file->offset >= offset && file->offset < offset + PAGE_SIZE) {
				entry = &cf->entries[i];
				break;
			}
		}
		if (!entry) {
			//load entry from fs, reacquire spinlocks and reload cf
			printk("Error reading file: unimplemented");
			error = -ENOSYS;
			goto ret;
			//continue;
		}
		entry->lastAccessed = jiffyCounter;
		entry->dirty = true;

		unsigned int diff = file->offset - offset;
		unsigned int nrofBytes = PAGE_SIZE - diff;
		memcpy((void *)((uintptr_t)(entry->addr) + diff), buffer, nrofBytes);

		buffer = (void *)((uintptr_t)buffer + nrofBytes);
		file->offset += nrofBytes;
		bufSize -= nrofBytes;
		if (file->offset > inode->fileSize) {
			inode->fileSize = file->offset;
		}
	}

	ret:
	releaseSpinlock(&inode->lock);
	releaseSpinlock(&file->lock);
	return error;
}

int fsSeek(struct File *file, int64_t offset, int whence) {
	acquireSpinlock(&file->lock);
	acquireSpinlock(&file->inode->lock);

	int64_t newOffset;
	int error = 0;
	switch (whence) {
		case SEEK_SET:
			if (offset < 0 || offset > (int64_t)file->inode->fileSize) {
				error = -EINVAL;
				break;
			}
			file->offset = offset;
			break;
		case SEEK_CUR:
			newOffset = file->offset + offset;
			if (newOffset < 0 || newOffset > (int64_t)file->inode->fileSize) {
				error = -EINVAL;
				break;
			} 
			file->offset = newOffset;
			break;
		case SEEK_END:
			newOffset = file->inode->fileSize + offset;
			if (newOffset < 0 || newOffset > (int64_t)file->inode->fileSize) {
				error = -EINVAL;
				break;
			} 
			file->offset = newOffset;
			break;
		default:
			error = -EINVAL;
			break;
	}

	releaseSpinlock(&file->inode->lock);
	releaseSpinlock(&file->lock);
	return error;
}
