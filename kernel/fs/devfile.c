#include <fs/devfile.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <sched/spinlock.h>

struct Inode *fsCreateCharDev(struct Inode *dir, const char *name, struct DevFileOps *ops, void *privateData) {
	struct DirEntry entry;
	
	entry.nameLen = strlen(name);
	if (entry.nameLen > 31) {
		entry.name = kmalloc(entry.nameLen + 1);
		if (!entry.name) {
			//return -ENOMEM;
			return NULL;
		}
		memcpy(entry.name, name, entry.nameLen);
		entry.name[entry.nameLen] = 0;
	} else {
		memcpy(entry.inlineName, name, entry.nameLen);
		entry.inlineName[entry.nameLen] = 0;
	}

	acquireSpinlock(&dir->lock);

	struct Inode *newInode = kmalloc(sizeof(struct Inode));

	memset(newInode, 0, sizeof(struct Inode));

	//newInode->inodeID = dir->superBlock->curInodeID;
	//dir->superBlock->curInodeID += 1;
	newInode->superBlock = dir->superBlock;
	newInode->type = ITYPE_CHAR;
	newInode->nrofLinks = 1;
	newInode->ramfs = RAMFS_DEVFILE;
	newInode->ops = ops;
	newInode->cachedData = privateData;
	newInode->attr.perm = 0660;

	entry.inode = newInode;

	struct DirEntry *newEntry = &entry;
	int error = dirCacheAdd(&newEntry, dir);

	releaseSpinlock(&dir->lock);

	if (error) {
		kfree(newInode);
		return NULL;
	} else {
		return newInode;
	}
}

