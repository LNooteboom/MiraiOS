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
#define ITYPE_CHAR	3

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define RAMFS_PRESENT	1 //always set if inode belongs to ramfs
#define RAMFS_INITRD	2 //set if file comes from initrd
#define RAMFS_DEVFILE	3

typedef int64_t ssize_t;

struct Inode;
struct DirEntry;
struct DevFileOps;

struct File {
	spinlock_t lock;
	struct Inode *inode;
	uint64_t offset;
};

struct UserDentry {
	struct Inode *inode;
	char name[256];
};

struct SuperBlock {
	unsigned int fsID;
	unsigned int curInodeID;
};

struct InodeAttributes {
	uint32_t ownerID;
	uint32_t groupID;
	uint16_t accessPermissions;

	time_t creationTime;
	time_t modificationTime;
	time_t accessTime;
};

struct Inode {
	//struct RbNode rbHeader; //value = (fsIndex << 32) | inodeIndex
	uint32_t inodeID;

	unsigned int type;
	unsigned int refCount;
	unsigned int nrofLinks;
	
	unsigned int ramfs;

	uint64_t fileSize; //unused for dirs

	struct SuperBlock *superBlock;

	spinlock_t lock; //used for both the inode and cachedData
	struct InodeAttributes attr;

	bool cacheDirty;
	void *cachedData;
	size_t cachedDataSize;

	struct DevFileOps *ops; //used for device files only
};

extern struct Inode *rootDir;

/*
Mounts the root directory
*/
int mountRoot(struct Inode *rootInode);

/*
Parses a file/directory path and returns the inode or NULL if not found
*/
struct Inode *getInodeFromPath(struct Inode *cwd, const char *path);

/*
Parses a file/directory path and returns the inode of the base directory and an index to the file name
Useful for fsCreate/fsLink.
*/
struct Inode *getBaseDirFromPath(struct Inode *cwd, int *fileNameIndex, const char *path);

/*
Opens a file
*/
int fsOpen(struct Inode *inode, struct File *output);

/*
Creates a file or directory
*/
int fsCreate(struct File *output, struct Inode *dir, const char *name, uint32_t type);

int fsGetDirEntry(struct Inode *dir, unsigned int index, struct UserDentry *dentry);

/*
Creates a directory entry for a specified inode
*/
int fsLink(struct Inode *dir, struct Inode *inode, const char *name);

/*
Removes a directory entry (and the inode if nrofLinks == 0)
*/
int fsUnlink(struct Inode *dir, const char *name);

/*
Reads from a file, returns nrof bytes read
*/
ssize_t fsRead(struct File *file, void *buffer, size_t bufSize);

/*
Writes to a file
*/
int fsWrite(struct File *file, void *buffer, size_t bufSize);

/*
Sets the current offset of a file stream
*/
int fsSeek(struct File *file, int64_t offset, int whence);

/*
Device-file specific function
*/
int fsIoctl(struct File *file, unsigned long request, ...);

/*
Gets directory entries
*/
int fsGetDents(struct Inode *dir, struct GetDents *buf, unsigned int nrofEntries);

#endif