#ifndef INCLUDE_FS_DIRENTRY_H
#define INCLUDE_FS_DIRENTRY_H

#include <sched/spinlock.h>
#include <stdint.h>
#include <stddef.h>

struct DirEntry {
	struct Inode *inode;
	struct Inode *parent;
	uint64_t unused;
	uint32_t index;
	uint32_t nameLen;
	union {
		char *name;
		char inlineName[32];
	};
};

struct CachedDir {
	unsigned int nrofEntries;

	struct DirEntry entries[2]; //can be more than 2, must be last
};

struct GetDents {
	struct Inode *inode;
	char name[256];
};

int dirCacheAdd(struct DirEntry **newEntry, struct Inode *dir);

int dirCacheRemove(struct DirEntry *entry);

struct DirEntry *dirCacheLookup(struct Inode *dir, const char *name);

int dirCacheDelete(struct Inode *dir);

int dirCacheInit(struct Inode *dir, struct Inode *parentDir);

int dirCacheList(struct Inode *dir, struct GetDents *buf, unsigned int nrofEntries);

#endif