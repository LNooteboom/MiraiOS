#include <fs/devfile.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <sched/spinlock.h>

int fsCreateCharDev(struct inode *dir, const char *name, struct devFileOps *ops) {
	struct dirEntry entry;
	
	entry.nameLen = strlen(name);
	if (entry.nameLen > 31) {
		entry.name = kmalloc(entry.nameLen + 1);
		if (!entry.name) {
			return -ENOMEM;
		}
		memcpy(entry.name, name, entry.nameLen);
		entry.name[entry.nameLen] = 0;
	} else {
		memcpy(entry.inlineName, name, entry.nameLen);
		entry.inlineName[entry.nameLen] = 0;
	}

	acquireSpinlock(&dir->lock);

	struct inode *newInode = kmalloc(sizeof(struct inode));

	memset(newInode, 0, sizeof(struct inode));

	//newInode->inodeID = dir->superBlock->curInodeID;
	//dir->superBlock->curInodeID += 1;
	newInode->superBlock = dir->superBlock;
	newInode->type = ITYPE_CHAR;
	newInode->attr.accessPermissions = dir->attr.accessPermissions;
	newInode->nrofLinks = 1;
	newInode->ramfs = RAMFS_DEVFILE;
	newInode->ops = ops;

	entry.inode = newInode;

	struct dirEntry *newEntry = &entry;
	int error = dirCacheAdd(&newEntry, dir);

	releaseSpinlock(&dir->lock);

	return error;
}

