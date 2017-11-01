#include <fs/fs.h>
#include <fs/direntry.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <print.h>

int dirCacheAdd(struct DirEntry **newEntry, struct Inode *dir) {
	//spinlock on dir must already be acquired, this applies to all functions in this file
	struct CachedDir *new;

	if (!dir->cachedData) {
		//load dircache from fs
		printk("Error creating dirCache: unimplemented");
		return -ENOSYS;
	} else {
		new = krealloc(dir->cachedData, dir->cachedDataSize + sizeof(struct DirEntry));
		if (!new) {
			return -ENOMEM;
		}
		dir->cachedDataSize += sizeof(struct DirEntry);
	}

	dir->cachedData = new;
	dir->cacheDirty = true;
	
	struct DirEntry *newEntry2 = &new->entries[new->nrofEntries];
	memcpy(newEntry2, *newEntry, sizeof(struct DirEntry));
	newEntry2->index = new->nrofEntries++;
	newEntry2->parent = dir;
	*newEntry = newEntry2;

	return 0;
}

int dirCacheRemove(struct DirEntry *entry) {
	struct Inode *dir = entry->parent;
	struct CachedDir *cd = dir->cachedData;

	memcpy(entry, entry + 1, (cd->nrofEntries - entry->index - 1) * sizeof(struct DirEntry)); //copy backwards so memcpy is safe

	cd->nrofEntries--;
	dir->cachedDataSize -= sizeof(struct DirEntry);
	dir->cachedData = krealloc(cd, dir->cachedDataSize);
	dir->cacheDirty = true;

	return 0;
}

struct DirEntry *dirCacheLookup(struct Inode *dir, const char *name) {
	if (!dir->cachedData) {
		return NULL;
	}
	struct CachedDir *cd = dir->cachedData;
	struct DirEntry *entry = NULL;
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
		if (memcmp(entryName, name, nameLen)) {
			continue;
		}
		//found
		entry = &cd->entries[i];
		break;
	}
	//entry is NULL if failed
	return entry;
}

int dirCacheDelete(struct Inode *dir) {
	struct CachedDir *cd = dir->cachedData;
	for (unsigned int i = 0; i < cd->nrofEntries; i++) {
		if (cd->entries[i].nameLen > 31) {
			kfree(cd->entries[i].name);
		}
	}
	kfree(cd);
	return 0;
}

int dirCacheList(struct Inode *dir, struct GetDents *buf, unsigned int nrofEntries) {
	struct CachedDir *cd = dir->cachedData;
	if (nrofEntries > cd->nrofEntries) {
		nrofEntries = cd->nrofEntries;
	}
	for (unsigned int i = 0; i < nrofEntries; i++) {
		struct DirEntry *curEntry = &cd->entries[i];
		buf[i].inode = curEntry->inode;
		if (curEntry->nameLen > 31) {
			memcpy(&buf[i].name, curEntry->name, curEntry->nameLen);
		} else {
			memcpy(&buf[i].name, curEntry->inlineName, curEntry->nameLen);
		}
		buf[i].name[curEntry->nameLen] = 0;
	}

	return nrofEntries;
}

int dirCacheInit(struct Inode *dir, struct Inode *parentDir) {
	struct CachedDir *cd = kmalloc(sizeof(struct CachedDir));
	if (!cd) {
		return -ENOMEM;
	}
	dir->cachedData = cd;
	dir->cachedDataSize = sizeof(struct CachedDir);

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