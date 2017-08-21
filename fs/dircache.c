#include <fs/fs.h>
#include <fs/direntry.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <print.h>

int dirCacheAdd(struct dirEntry **newEntry, struct inode *dir) {
	//spinlock on dir must already be acquired
	struct cachedDir *new;

	if (!dir->cachedData) {
		if (dir->ramfs) {
			//dir cache was created new, initialize cachedDir struct
			new = kmalloc(sizeof(struct cachedDir));
			if (!new) {
				return -ENOMEM;
			}
			dir->cachedDataSize = sizeof(struct cachedDir);
			new->nrofEntries = 0;
		} else {
			//load dircache from fs
			printk("Error creating dirCache: unimplemented");
			return -ENOSYS;
		}
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
	*newEntry = newEntry2;

	return 0;
}

int dirCacheRemove(struct dirEntry *entry) {
	struct inode *dir = entry->parent;

	acquireSpinlock(&dir->lock);

	struct cachedDir *cd = dir->cachedData;

	memcpy(entry, entry + 1, (cd->nrofEntries - entry->index - 1) * sizeof(struct dirEntry)); //copy backwards so memcpy is safe

	cd->nrofEntries--;
	if (!cd->nrofEntries) {
		kfree(cd);
		dir->cachedData = NULL;
		dir->cachedDataSize = 0;
	} else {
		dir->cachedDataSize -= sizeof(struct dirEntry);
		dir->cachedData = krealloc(cd, dir->cachedDataSize - sizeof(struct dirEntry));
	}
	dir->cacheDirty = true;

	releaseSpinlock(&dir->lock);

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