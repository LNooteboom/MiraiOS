#ifndef RAMFS_H
#define RAMFS_H

#include <fs/fs.h>

struct ramfsInode {
	struct inode base;
	void *fileAddr;
	unsigned long nrofPages;
};

struct dirEntry *ramfsLookup(struct inode *dir, const char *name);
int _ramfsCreate(struct inode **out, struct inode *dir, const char *name, uint32_t type);

extern struct superBlock ramfsSuperBlock;
extern struct fileOps ramfsFileOps;
extern struct inodeOps ramfsInodeOps;

#endif