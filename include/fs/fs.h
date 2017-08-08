#ifndef INCLUDE_FS_INODE_H
#define INCLUDE_FS_INODE_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <lib/rbtree.h>
#include <sched/spinlock.h>

#define ITYPE_MASK	7
#define ITYPE_DIR	1
#define ITYPE_FILE	2

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

typedef int64_t ssize_t;

struct inode;
struct dirEntry;

struct file {
	struct dirEntry *path;
	struct inode *inode;

	const struct fileOps *fOps;

	unsigned int refCount;
	spinlock_t lock;
	uint64_t offset;
};

struct superBlock {
	unsigned int fsID;
	unsigned int curInodeID;
};

struct inodeAttributes {
	uint32_t ownerID;
	uint32_t groupID;
	uint16_t accessPermissions;

	time_t creationTime;
	time_t modificationTime;
	time_t accessTime;
};

struct inodeOps {
	//directory operations
	int (*create)(struct inode *dir, const char *name, uint32_t type);
	int (*link)(struct inode *dir, struct inode *inode, const char *name);
	int (*unlink)(struct inode *dir, const char *name);
};

struct fileOps {
	int (*open)(struct inode *dir, struct file *output, const char *name);
	ssize_t (*read)(struct file *file, void *buffer, size_t bufSize);
	int (*write)(struct file *file, void *buffer, size_t bufSize);
	int (*seek)(struct file *file, int64_t offset, int whence);
};

struct inode {
	struct rbNode rbHeader; //value = (fsIndex << 32) | inodeIndex

	unsigned int type;
	unsigned int refCount;
	unsigned int nrofLinks;

	uint64_t fileSize;

	const struct superBlock *superBlock;
	const struct inodeOps *iOps;
	const struct fileOps *fOps;

	spinlock_t lock;
	struct inodeAttributes attr;

	//void *privateData;
};

struct dirEntry { //64 bytes
	struct inode *inode;
	struct inode *parent;
	struct superBlock *superBlock;
	uint32_t refCount;
	uint32_t nameLen;
	union {
		char *name;
		char inlineName[32];
	};
};

extern struct dirEntry rootDir;

int fsAddInode(struct inode *inode);
int fsDeleteInode(struct inode *inode);
int mountRoot(struct inode *rootInode);

#endif