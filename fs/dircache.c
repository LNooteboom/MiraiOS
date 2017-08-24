#include <fs/fs.h>
#include <fs/direntry.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <print.h>

int dirCacheAdd(struct dirEntry **newEntry, struct inode *dir) {
	//spinlock on dir must already be acquired, this applies to all functions in this file
	struct cachedDir *new;

	if (!dir->cachedData) {
		//load dircache from fs
		printk("Error creating dirCache: unimplemented");
		return -ENOSYS;
	} else {
		new = krealloc(dir->cachedData, dir->cachedDataSize + sizeof(struct dirEntry));
		if (!new) {
			return -ENOMEM;
		}
		dir->cachedDataSize += sizeof(struct dirEntry);
	}

	dir->cachedData = new;
	dir->cacheDirty = true;
	
	struct dirEntry *newEntry2 = &new->entries[new->nrofEntries];
	memcpy(newEntry2, *newEntry, sizeof(struct dirEntry));
	newEntry2->index = new->nrofEntries++;
	newEntry2->parent = dir;
	*newEntry = newEntry2;

	return 0;
}

int dirCacheRemove(struct dirEntry *entry) {
	struct inode *dir = entry->parent;
	struct cachedDir *cd = dir->cachedData;

	memcpy(entry, entry + 1, (cd->nrofEntries - entry->index - 1) * sizeof(struct dirEntry)); //copy backwards so memcpy is safe

	cd->nrofEntries--;
	dir->cachedDataSize -= sizeof(struct dirEntry);
	dir->cachedData = krealloc(cd, dir->cachedDataSize);
	dir->cacheDirty = true;

	return 0;
}

struct dirEntry *dirCacheLookup(struct inode *dir, const char *name) {
	if (!dir->cachedData) {
		return NULL;
	}
	struct cachedDir *cd = dir->cachedData;
	struct dirEntry *entry = NULL;
	size_t nameLen = strlen(name);
	for (unsigned int i = 0; i < cd->nrofEntries; i++) {
		if (cd->entries[i].nameLen != nameLen) {
			continue;
		}
		const char *entryName;
		if (nameLen > 31) {
			entryName = cd->entries[i].name;
		} else {
			entryName = cd->entries[i].inlineName;
		}
		if (!memcmp(entryName, name, nameLen)) {
			continue;
		}
		//found
		entry = &cd->entries[i];
		break;
	}
	//entry is NULL if failed
	return entry;
}

int dirCacheDelete(struct inode *dir) {
	struct cachedDir *cd = dir->cachedData;
	for (unsigned int i = 0; i < cd->nrofEntries; i++) {
		if (cd->entries[i].nameLen > 31) {
			kfree(cd->entries[i].name);
		}
	}
	kfree(cd);
	return 0;
}

int dirCacheInit(struct inode *dir, struct inode *parentDir) {
	struct cachedDir *cd = kmalloc(sizeof(struct cachedDir));
	if (!cd) {
		return -ENOMEM;
	}
	dir->cachedData = cd;
	dir->cachedDataSize = sizeof(struct cachedDir);

	//"." entry, points to same dir
	cd->nrofEntries = 2;
	cd->entries[0].inode = dir;
	cd->entries[0].parent = dir;
	cd->entries[0].index = 0;
	cd->entries[0].nameLen = 1;
	cd->entries[0].inlineName[0] = '.';
	cd->entries[0].inlineName[1] = 0;

	//".." entry¸ points to parent dir
	cd->entries[1].inode = parentDir;
	cd->entries[1].parent = dir;
	cd->entries[1].index = 0;
	cd->entries[1].nameLen = 2;
	cd->entries[1].inlineName[0] = '.';
	cd->entries[1].inlineName[1] = '.';
	cd->entries[1].inlineName[2] = 0;

	return 0;
}