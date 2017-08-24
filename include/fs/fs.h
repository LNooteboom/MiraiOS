#ifndef INCLUDE_FS_INODE_H
#define INCLUDE_FS_INODE_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <lib/rbtree.h>
#include <sched/spinlock.h>
#include <fs/direntry.h>

#define ITYPE_MASK	7
#define ITYPE_DIR	1
#define ITYPE_FILE	2

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define RAMFS_PRESENT	1 //always set if inode belongs to ramfs
#define RAMFS_INITRD	2 //set if file comes from initrd

typedef int64_t ssize_t;

struct inode;
struct dirEntry;

struct file {
	spinlock_t lock;
	struct inode *inode;
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

struct dirOps {
	//directory operations
	struct inode *(*lookup)(struct inode *dir, const char *name);
	int (*create)(struct inode *dir, const char *name, uint32_t type);
	int (*open)(struct inode *file, struct file *output);
	int (*link)(struct inode *dir, struct inode *inode, const char *name);
	int (*unlink)(struct inode *dir, const char *name);
};

struct fileOps {
	ssize_t (*read)(struct file *file, void *buffer, size_t bufSize);
	int (*write)(struct file *file, void *buffer, size_t bufSize);
	int (*seek)(struct file *file, int64_t offset, int whence);
};

struct inode {
	//struct rbNode rbHeader; //value = (fsIndex << 32) | inodeIndex
	uint32_t inodeID;

	unsigned int type;
	unsigned int refCount;
	unsigned int nrofLinks;
	
	unsigned int ramfs;

	uint64_t fileSize; //unused for dirs

	struct superBlock *superBlock;

	spinlock_t lock; //used for both the inode and cachedData
	struct inodeAttributes attr;

	bool cacheDirty;
	void *cachedData;
	size_t cachedDataSize;
};

extern struct inode *rootDir;

/*
Mounts the root directory
*/
int mountRoot(struct inode *rootInode);

/*
Parses a file/directory path and returns the inode or NULL if not found
*/
struct inode *getInodeFromPath(struct inode *cwd, const char *path);

/*
Parses a file/directory path and returns the inode of the base directory and an index to the file name
Useful for fsCreate/fsLink.
*/
struct inode *getBaseDirFromPath(struct inode *cwd, int *fileNameIndex, const char *path);

/*
Opens a file
*/
int fsOpen(struct inode *inode, struct file *output);

/*
Creates a file or directory
*/
int fsCreate(struct file *output, struct inode *dir, const char *name, uint32_t type);

/*
Creates a directory entry for a specified inode
*/
int fsLink(struct inode *dir, struct inode *inode, const char *name);

/*
Removes a directory entry (and the inode if nrofLinks == 0)
*/
int fsUnlink(struct inode *dir, const char *name);

/*
Reads from a file, returns nrof bytes read
*/
ssize_t fsRead(struct file *file, void *buffer, size_t bufSize);

/*
Writes to a file
*/
int fsWrite(struct file *file, void *buffer, size_t bufSize);

/*
Sets the current offset of a file stream
*/
int fsSeek(struct file *file, int64_t offset, int whence);

#endif