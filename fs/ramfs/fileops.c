#include <fs/fs.h>
#include "ramfs.h"
#include <errno.h>
#include <mm/paging.h>
#include <mm/memset.h>
#include <mm/pagemap.h>

static int ramfsOpen(struct inode *dir, struct file *output, const char *name);
static ssize_t ramfsRead(struct file *file, void *buffer, size_t bufSize);
static int ramfsWrite(struct file *file, void *buffer, size_t bufSize);
static int ramfsSeek(struct file *file, int64_t offset, int whence);

struct fileOps ramfsFileOps = {
	.open = ramfsOpen,
	.read = ramfsRead,
	.write = ramfsWrite,
	.seek = ramfsSeek
};

static int ramfsOpen(struct inode *dir, struct file *output, const char *name) {
	struct dirEntry *entry = ramfsLookup(dir, name);
	if (!entry) {
		return -ENOENT;
	}
	output->path = entry;
	output->inode = entry->inode;
	output->fOps = &ramfsFileOps;
	output->refCount = 1;
	output->lock = 0;
	output->offset = 0;
	return 0;
}

static ssize_t ramfsRead(struct file *file, void *buffer, size_t bufSize) {
	struct ramfsInode *inode = (struct ramfsInode *)file->inode;
	size_t bytesLeft = inode->base.fileSize - file->offset;
	if (bufSize > bytesLeft) {
		bufSize = bytesLeft;
	}
	char *begin = &((char *)inode->fileAddr)[file->offset];
	memcpy(buffer, begin, bufSize);
	file->offset += bufSize;
	return bytesLeft;
}

static int ramfsWrite(struct file *file, void *buffer, size_t bufSize) {
	struct ramfsInode *inode = (struct ramfsInode *)file->inode;
	uint32_t pageSpaceLeft = 0;
	if (inode->base.fileSize) {
		pageSpaceLeft = (inode->nrofPages * PAGE_SIZE) - inode->base.fileSize;
	}
	if (bufSize > pageSpaceLeft) {
		unsigned long nrofNewPages = (bufSize - pageSpaceLeft) / PAGE_SIZE;
		if ((bufSize - pageSpaceLeft) % PAGE_SIZE) {
			nrofNewPages++;
		}
		unsigned long nrofOldPages = inode->nrofPages;
		void *newAddr = allocKPages((nrofOldPages + nrofNewPages) * PAGE_SIZE, 0);
		if (!newAddr) {
			return -ENOMEM;
		}
		uintptr_t curAddr = (uintptr_t)newAddr;
		//copy page mappings
		for (unsigned long i = 0; i < nrofOldPages; i++) {
			physPage_t phys = mmGetPageEntry((uintptr_t)inode->fileAddr + i * PAGE_SIZE);
			mmMapPage(curAddr, phys, PAGE_FLAG_WRITE);
			curAddr += PAGE_SIZE;
		}
		//map new pages
		for (unsigned long i = 0; i < nrofNewPages; i++) {
			mmSetPageFlags(curAddr, PAGE_FLAG_WRITE | PAGE_FLAG_INUSE);
			curAddr += PAGE_SIZE;
		}
		//clear old mapping
		for (unsigned long i = 0; i < nrofOldPages; i++) {
			mmUnmapPage((uintptr_t)inode->fileAddr + i * PAGE_SIZE);
		}
		inode->fileAddr = newAddr;
		inode->nrofPages = nrofOldPages + nrofNewPages;
	}
	
	memcpy(&((char *)inode->fileAddr)[inode->base.fileSize], buffer, bufSize);
	inode->base.fileSize += bufSize;
	file->offset += bufSize;
	return 0;
}

static int ramfsSeek(struct file *file, int64_t offset, int whence) {
	int64_t newOffset;
	switch (whence) {
		case SEEK_SET:
			if (offset < 0 || offset > (int64_t)file->inode->fileSize) {
				return -EINVAL;
			}
			file->offset = offset;
			break;
		case SEEK_CUR:
			newOffset = file->offset + offset;
			if (newOffset < 0 || newOffset > (int64_t)file->inode->fileSize) {
				return -EINVAL;
			} 
			file->offset = newOffset;
			break;
		case SEEK_END:
			newOffset = file->inode->fileSize + offset;
			if (newOffset < 0 || newOffset > (int64_t)file->inode->fileSize) {
				return -EINVAL;
			} 
			file->offset = newOffset;
			break;
		default:
			return -EINVAL;
	}
	return 0;
}