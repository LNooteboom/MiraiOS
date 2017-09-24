#include <fs/fs.h>
#include <stdint.h>
#include <stddef.h>
#include <arch/bootinfo.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <mm/memset.h>
#include <print.h>
#include <modules.h>

struct CpioHeader {
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

struct SuperBlock ramfsSuperBlock = {
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

static int parseInitrd(struct Inode *rootInode) {
	//char *initrd = bootInfo.initrd;
	char *initrd = ioremap((uintptr_t)bootInfo.initrd, bootInfo.initrdLen);
	struct CpioHeader *initrdHeader;
	unsigned long curPosition = 0;
	while (curPosition < bootInfo.initrdLen) {
		initrdHeader = (struct CpioHeader *)(&initrd[curPosition]);
		if (!memcmp(initrdHeader->magic, cpioMagic, 6)) {
			printk("Invalid CPIO header: %s\n", initrdHeader->magic);
			return -EINVAL;
		}
		uint32_t nameLen = parseHex(initrdHeader->namesize);
		char *name = &initrd[curPosition + sizeof(struct CpioHeader)];
		if (nameLen == sizeof(cpioEndName) && memcmp(name, cpioEndName, sizeof(cpioEndName))) {
			break;
		}
		
		//TODO add support for directories (and hardlinks?)

		printk("[INITRD] Found file: %s\n", name);
		struct Inode *newInode = kmalloc(sizeof(struct Inode));
		memset(newInode, 0, sizeof(struct Inode));

		newInode->cachedData = name + nameLen;
		uint32_t fileSize = parseHex(initrdHeader->filesize);
		newInode->fileSize = fileSize;
		newInode->ramfs = RAMFS_PRESENT | RAMFS_INITRD;
		newInode->type = ITYPE_FILE;
		newInode->superBlock = &ramfsSuperBlock;
		rootInode->attr.accessPermissions = 0664;

		int error = fsLink(rootInode, newInode, name);
		if (error) {
			return error;
		}

		curPosition += sizeof(struct CpioHeader) + nameLen + fileSize;
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
	struct Inode *rootInode = kmalloc(sizeof(struct Inode));
	if (!rootInode) {
		return -ENOMEM;
	}
	memset(rootInode, 0, sizeof(struct Inode));

	rootInode->inodeID = 1;
	rootInode->type = ITYPE_DIR;
	rootInode->refCount = 1;
	rootInode->nrofLinks = 1;
	rootInode->ramfs = RAMFS_PRESENT;
	rootInode->superBlock = &ramfsSuperBlock;
	rootInode->attr.accessPermissions = 0664;

	dirCacheInit(rootInode, rootInode);

	mountRoot(rootInode);
	ramfsSuperBlock.curInodeID = 2;

	return parseInitrd(rootInode);
}

MODULE_INIT_LEVEL(ramfsInit, 2);