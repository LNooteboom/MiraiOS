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
char cpioEndName[10] = "TRAILER!!!";

static int ramfsCreate(struct inode **out, struct inode *dir, struct dirEntry *entry, uint32_t type);
/*static int ramfsLink(struct inode *dir, struct inode *inode, struct dirEntry *newEntry);
static int ramfsUnlink(struct inode *dir, struct dirEntry *entry);*/

static int ramfsOpen(struct file *output, struct dirEntry *entry);
static ssize_t ramfsRead(struct file *file, void *buffer, size_t bufSize);
static ssize_t ramfsWrite(struct file *file, void *buffer, size_t bufSize);

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
	.write = ramfsWrite
};

static int ramfsCreateDirEnt(struct inode *dir, struct dirEntry *entry) {
	struct ramfsInode *dir2 = (struct ramfsInode *)dir;
	asm ("xchg bx, bx");
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

static int ramfsCreate(struct inode **out, struct inode *dir, struct dirEntry *entry, uint32_t type) {
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

	entry->inode = (struct inode *)newInode;
	fsAddInode((struct inode *)newInode);
	int error;
	if (error = ramfsCreateDirEnt(dir, entry)) {
		kfree(newInode);
		return error;
	}
	if (out) {
		*out = (struct inode *)newInode;
	}
	return 0;
}
/*
static int ramfsLink(struct inode *dir, struct inode *inode, struct dirEntry *newEntry) {
	return 0;
}

static int ramfsUnlink(struct inode *dir, struct dirEntry *entry) {
	return 0;
}*/

static int ramfsOpen(struct file *output, struct dirEntry *entry) {
	output->path = entry;
	if (!entry->inode) {
		return -EINVAL;
	}
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
	return bytesLeft;
}

static ssize_t ramfsWrite(struct file *file, void *buffer, size_t bufSize) {
	return 0;
}

static uint32_t parseHex(char *str) {
	uint32_t result = 0;
	for (int i = 0; i < 8; i++) {
		if (str[i] > 'A') {
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
		if (!memcmp(&initrdHeader->magic, cpioMagic, 6)) {
			printk("Invalid CPIO header!");
			return -EINVAL;
		}
		uint32_t nameLen = parseHex(initrdHeader->namesize);
		char *name = &initrd[curPosition + sizeof(struct cpioHeader)];
		if (nameLen == 10 && memcmp(name, cpioEndName, 10)) {
			break;
		}
		//TODO add support for directories and hardlinks

		//create dir entry
		struct dirEntry dEnt;
		if (nameLen > 31) {
			char *entryName = kmalloc(nameLen + 1);
			if (!entryName) {
				return -ENOMEM;
			}
			memcpy(entryName, name, nameLen);
			entryName[nameLen] = 0; //add null terminator
			dEnt.name = entryName;
			printk("Add file: %s\n", dEnt.name);
		} else {
			if (memcmp(name, cpioEndName, sizeof(cpioEndName))) {
				break;
			}
			//use inline name
			memcpy(dEnt.inlineName, name, nameLen);
			dEnt.inlineName[nameLen] = 0;
			printk("Add file (inline): %s\n", name);
		}
		dEnt.nameLen = nameLen;
		struct ramfsInode *newInode;
		int error = ramfsCreate((struct inode **)(&newInode), (struct inode *)rootInode, &dEnt, ITYPE_FILE);
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
	//initialize initrd
	//create root directory
	/*struct dirEntry *rootContents = allocKPages(PAGE_SIZE, PAGE_FLAG_WRITE | PAGE_FLAG_CLEAN);
	if (!rootContents) {
		return -ENOMEM;
	}*/
	//create root inode
	struct ramfsInode *rootInode = kmalloc(sizeof(struct ramfsInode));
	if (!rootInode) {
		//deallocPages(rootContents, PAGE_SIZE);
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