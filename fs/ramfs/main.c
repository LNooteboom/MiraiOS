#include <fs/fs.h>
#include "ramfs.h"
#include <arch/bootinfo.h>
#include <errno.h>
#include <print.h>
#include <mm/memset.h>
#include <mm/heap.h>

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

static char cpioMagic[6] = "070701";
static char cpioEndName[] = "TRAILER!!!";

struct superBlock ramfsSuperBlock = {
	.fsID = 1
};

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