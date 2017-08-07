#include <fs/fs.h>
#include <arch/bootinfo.h>
#include <mm/paging.h>
#include <mm/memset.h>
#include <mm/pagemap.h>
#include <mm/heap.h>
#include <errno.h>

#include <print.h>

struct ramfsInode {
	struct inode base;
	void *fileAddr;
	unsigned long nrofPages;
};

struct cpioHeader {
	char magic[6];
	char ino[8];
	char mode[8];
	char uid[8];
	char gid[8];
	char nlink[8];
	char mtime[8];
	char filesize[8];
	char devmajor[8];
	char devminor[8];
	char rdevmajor[8];
	char rdevminor[8];
	char namesize[8];
	char check[8];
};

char cpioMagic[6] = "070701";
char cpioEndName[] = "TRAILER!!!";

static int ramfsCreate(struct inode *dir, const char *name, uint32_t type);
/*static int ramfsLink(struct inode *dir, struct inode *inode, struct dirEntry *newEntry);
static int ramfsUnlink(struct inode *dir, struct dirEntry *entry);*/

static int ramfsOpen(struct inode *dir, struct file *output, const char *name);
static ssize_t ramfsRead(struct file *file, void *buffer, size_t bufSize);
static int ramfsWrite(struct file *file, void *buffer, size_t bufSize);
static int ramfsSeek(struct file *file, int64_t offset, int whence);

static struct superBlock ramfsSuperBlock = {
	.fsID = 1
};

static struct inodeOps ramfsInodeOps = {
	.create = ramfsCreate,
	//.link = ramfsLink,
	//.unlink = ramfsUnlink
};

static struct fileOps ramfsFileOps = {
	.open = ramfsOpen,
	.read = ramfsRead,
	.write = ramfsWrite,
	.seek = ramfsSeek
};

static int ramfsCreateDirEnt(struct inode *dir, struct dirEntry *entry) {
	struct ramfsInode *dir2 = (struct ramfsInode *)dir;
	if (!dir2 || (dir2->base.type & ITYPE_MASK) != ITYPE_DIR) {
		return -EINVAL;
	}
	acquireSpinlock(&dir2->base.lock);
	if (!dir2->fileAddr) {
		dir2->fileAddr = allocKPages(PAGE_SIZE, PAGE_FLAG_INUSE | PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE);
		if (!dir2->fileAddr) {
			return -ENOMEM;
		}
		dir2->nrofPages = 1;
	} else if (dir2->base.fileSize + sizeof(struct dirEntry) > dir2->nrofPages * PAGE_SIZE) {
		//allocate another page
		uintptr_t pageAddr = (uintptr_t)(dir2->fileAddr) + (dir2->nrofPages * PAGE_SIZE);
		if (mmGetPageEntry(pageAddr)) {
			//next page already exists
			return -ENOMEM;
		}
		allocPageAt((void *)pageAddr, PAGE_SIZE, PAGE_FLAG_INUSE | PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE);
		dir2->nrofPages++;
	}
	unsigned int nrofDirEntries = dir2->base.fileSize / sizeof(struct dirEntry);
	struct dirEntry *dirEnts = dir2->fileAddr;
	//now copy direntry
	memcpy(&dirEnts[nrofDirEntries], entry, sizeof(struct dirEntry));
	dirEnts[nrofDirEntries].parent = (struct inode *)dir2;
	dir2->base.fileSize += sizeof(struct dirEntry);
	releaseSpinlock(&dir2->base.lock);
	return 0;
}

static int _ramfsCreate(struct inode **out, struct inode *dir, const char *name, uint32_t type) {
	struct dirEntry entry;
	entry.nameLen = strlen(name);
	if (entry.nameLen > 31) {
		entry.name = kmalloc(entry.nameLen + 1);
		if (!entry.name) {
			return -ENOMEM;
		}
		memcpy(entry.name, name, entry.nameLen);
		entry.name[entry.nameLen] = 0;
	} else {
		memcpy(entry.inlineName, name, entry.nameLen);
		entry.inlineName[entry.nameLen] = 0;
	}
	

	struct ramfsInode *newInode = kmalloc(sizeof(struct ramfsInode));
	if (!newInode) {
		return -ENOMEM;
	}

	memset(newInode, 0, sizeof(struct ramfsInode));
	newInode->base.rbHeader.value = ((uint64_t)ramfsSuperBlock.fsID << 32) | ramfsSuperBlock.curInodeID++;
	newInode->base.type = type;
	newInode->base.iOps = &ramfsInodeOps;
	newInode->base.fOps = &ramfsFileOps;
	newInode->base.attr.accessPermissions = 0664;

	entry.inode = (struct inode *)newInode;
	fsAddInode((struct inode *)newInode);
	int error = ramfsCreateDirEnt(dir, &entry);
	if (error) {
		kfree(newInode);
		return error;
	}
	if (out) {
		*out = (struct inode *)newInode;
	}
	return 0;
}

static int ramfsCreate(struct inode *dir, const char *name, uint32_t type) {
	return _ramfsCreate(NULL, dir, name, type);
}
/*
static int ramfsLink(struct inode *dir, struct inode *inode, struct dirEntry *newEntry) {
	return 0;
}

static int ramfsUnlink(struct inode *dir, struct dirEntry *entry) {
	return 0;
}*/

static int ramfsOpen(struct inode *dir, struct file *output, const char *name) {
	struct dirEntry *entry = NULL;
	if (!dir) {
		//get root dir
		entry = &rootDir;
	} else {
		size_t nameLen = strlen(name);
		struct dirEntry *dirEntries = ((struct ramfsInode *)dir)->fileAddr;
		for (unsigned int i = 0; i < dir->fileSize / sizeof(struct dirEntry); i++) {
			if (dirEntries[i].nameLen != nameLen) {
				continue;
			}
			const char *entryName;
			if (nameLen > 31) {
				entryName = dirEntries[i].name;
			} else {
				entryName = dirEntries[i].inlineName;
			}
			if (!memcmp(entryName, name, nameLen)) {
				continue;
			}
			//found
			entry = &dirEntries[i];
			break;
		}
		if (!entry) {
			return -ENOENT;
		}
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

static uint32_t parseHex(char *str) {
	uint32_t result = 0;
	for (int i = 0; i < 8; i++) {
		if (str[i] >= 'A') {
			result += (str[i] - 'A' + 10) << (28 - (i*4));
		} else {
			result += (str[i] - '0') << (28 - (i*4));
		}
	}
	return result;
}

static int parseInitrd(struct ramfsInode *rootInode) {
	//char *initrd = bootInfo.initrd;
	char *initrd = ioremap((uintptr_t)bootInfo.initrd, bootInfo.initrdLen);
	struct cpioHeader *initrdHeader;
	unsigned long curPosition = 0;
	while (curPosition < bootInfo.initrdLen) {
		initrdHeader = (struct cpioHeader *)(&initrd[curPosition]);
		if (!memcmp(initrdHeader->magic, cpioMagic, 6)) {
			printk("Invalid CPIO header: %s\n", initrdHeader->magic);
			return -EINVAL;
		}
		uint32_t nameLen = parseHex(initrdHeader->namesize);
		char *name = &initrd[curPosition + sizeof(struct cpioHeader)];
		if (nameLen == sizeof(cpioEndName) && memcmp(name, cpioEndName, sizeof(cpioEndName))) {
			break;
		}
		
		//TODO add support for directories and hardlinks

		printk("Add file: %s\n", name);
		struct ramfsInode *newInode;
		int error = _ramfsCreate((struct inode **)(&newInode), (struct inode *)rootInode, name, ITYPE_FILE);
		if (error) {
			return error;
		}
		newInode->fileAddr = name + nameLen;
		uint32_t fileSize = parseHex(initrdHeader->filesize);
		newInode->base.fileSize = fileSize;
		curPosition += sizeof(struct cpioHeader) + nameLen + fileSize;
		if (curPosition & 3) {
			curPosition &= ~3;
			curPosition += 4;
		}
	}
	return 0;
}

int ramfsInit(void) {
	//register driver

	if (!bootInfo.initrd) {
		return 0;
	}
	//create root inode
	struct ramfsInode *rootInode = kmalloc(sizeof(struct ramfsInode));
	if (!rootInode) {
		return -ENOMEM;
	}
	memset(rootInode, 0, sizeof(struct ramfsInode));
	rootInode->base.type = ITYPE_DIR;
	rootInode->base.iOps = &ramfsInodeOps;
	rootInode->base.fOps = &ramfsFileOps;
	rootInode->base.attr.accessPermissions = 0664;
	rootInode->base.rbHeader.value = ((uint64_t)ramfsSuperBlock.fsID << 32) | 1;
	mountRoot(&rootInode->base);
	ramfsSuperBlock.curInodeID = 2;

	return parseInitrd(rootInode);
}