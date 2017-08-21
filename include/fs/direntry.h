#ifndef INCLUDE_FS_DIRENTRY_H
#define INCLUDE_FS_DIRENTRY_H

#include <sched/spinlock.h>
#include <stdint.h>
#include <stddef.h>

struct dirEntry {
	struct inode *inode;
	struct inode *parent;
	struct superBlock *superBlock;
	uint32_t index;
	uint32_t nameLen;
	union {
		char *name;
		char inlineName[32];
	};
};

struct cachedDir {
	unsigned int nrofEntries;

	struct dirEntry entries[1]; //must be last
};

int dirCacheAdd(struct dirEntry **newEntry, struct inode *dir);

int dirCacheRemove(struct dirEntry *entry);

struct dirEntry *dirCacheLookup(struct inode *dir, const char *name);

int dirCacheDelete(struct inode *dir);

#endif