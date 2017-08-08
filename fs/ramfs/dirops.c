#include <fs/fs.h>
#include "ramfs.h"
#include <mm/paging.h>
#include <mm/memset.h>
#include <mm/pagemap.h>
#include <mm/heap.h>
#include <errno.h>

static int ramfsCreate(struct inode *dir, const char *name, uint32_t type);
static int ramfsLink(struct inode *dir, struct inode *inode, const char *name);
static int ramfsUnlink(struct inode *dir, const char *name);

struct inodeOps ramfsInodeOps = {
	.create = ramfsCreate,
	.link = ramfsLink,
	.unlink = ramfsUnlink
};

static int ramfsCreateDirEnt(struct inode *dir, struct dirEntry *entry) {
	struct ramfsInode *dir2 = (struct ramfsInode *)dir;
	if (!dir2 || (dir2->base.type & ITYPE_MASK) != ITYPE_DIR) {
		return -EINVAL;
	}
	acquireSpinlock(&dir2->base.lock);
	if (!dir2->fileAddr) {
		dir2->fileAddr = allocKPages(PAGE_SIZE, PAGE_FLAG_INUSE | PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE);
		if (!dir2->fileAddr) {
			return -ENOMEM;
		}
		dir2->nrofPages = 1;
	} else if (dir2->base.fileSize + sizeof(struct dirEntry) > dir2->nrofPages * PAGE_SIZE) {
		//allocate another page
		uintptr_t pageAddr = (uintptr_t)(dir2->fileAddr) + (dir2->nrofPages * PAGE_SIZE);
		if (mmGetPageEntry(pageAddr)) {
			//next page already exists
			return -ENOMEM;
		}
		allocPageAt((void *)pageAddr, PAGE_SIZE, PAGE_FLAG_INUSE | PAGE_FLAG_CLEAN | PAGE_FLAG_WRITE);
		dir2->nrofPages++;
	}
	unsigned int nrofDirEntries = dir2->base.fileSize / sizeof(struct dirEntry);
	struct dirEntry *dirEnts = dir2->fileAddr;
	//now copy direntry
	memcpy(&dirEnts[nrofDirEntries], entry, sizeof(struct dirEntry));
	dirEnts[nrofDirEntries].parent = (struct inode *)dir2;
	dir2->base.fileSize += sizeof(struct dirEntry);
	releaseSpinlock(&dir2->base.lock);
	return 0;
}

struct dirEntry *ramfsLookup(struct inode *dir, const char *name) {
	struct dirEntry *entry = NULL;
	if (!dir) {
		//get root dir
		entry = &rootDir;
	} else {
		size_t nameLen = strlen(name);
		struct dirEntry *dirEntries = ((struct ramfsInode *)dir)->fileAddr;
		for (unsigned int i = 0; i < dir->fileSize / sizeof(struct dirEntry); i++) {
			if (dirEntries[i].nameLen != nameLen) {
				continue;
			}
			const char *entryName;
			if (nameLen > 31) {
				entryName = dirEntries[i].name;
			} else {
				entryName = dirEntries[i].inlineName;
			}
			if (!memcmp(entryName, name, nameLen)) {
				continue;
			}
			//found
			entry = &dirEntries[i];
			break;
		}
		//entry is NULL if failed
	}
	return entry;
}

int _ramfsCreate(struct inode **out, struct inode *dir, const char *name, uint32_t type) {
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

	struct ramfsInode *newInode = kmalloc(sizeof(struct ramfsInode));
	if (!newInode) {
		return -ENOMEM;
	}

	memset(newInode, 0, sizeof(struct ramfsInode));
	newInode->base.rbHeader.value = ((uint64_t)ramfsSuperBlock.fsID << 32) | ramfsSuperBlock.curInodeID++;
	newInode->base.type = type;
	newInode->base.iOps = &ramfsInodeOps;
	newInode->base.fOps = &ramfsFileOps;
	newInode->base.attr.accessPermissions = 0664;
	newInode->base.nrofLinks = 1;

	entry.inode = (struct inode *)newInode;
	fsAddInode((struct inode *)newInode);
	int error = ramfsCreateDirEnt(dir, &entry);
	if (error) {
		kfree(newInode);
		return error;
	}
	if (out) {
		*out = (struct inode *)newInode;
	}
	return 0;
}

static int ramfsCreate(struct inode *dir, const char *name, uint32_t type) {
	return _ramfsCreate(NULL, dir, name, type);
}

static int ramfsLink(struct inode *dir, struct inode *inode, const char *name) {
	struct dirEntry entry;
	size_t nameLen = strlen(name);
	if (nameLen <= 31) {
		memcpy(entry.inlineName, name, nameLen);
		entry.inlineName[nameLen] = 0;
	} else {
		entry.name = kmalloc(nameLen + 1);
		if (!entry.name) {
			return -ENOMEM;
		}
		memcpy(entry.name, name, nameLen);
		entry.name[nameLen] = 0;
	}
	entry.nameLen = nameLen;
	entry.inode = inode;
	entry.superBlock = &ramfsSuperBlock;
	inode->nrofLinks += 1;
	ramfsCreateDirEnt(dir, &entry);
	
	return 0;
}

static int ramfsUnlink(struct inode *dir, const char *name) {
	struct dirEntry *entry = ramfsLookup(dir, name);
	if (!entry) {
		return -ENOENT;
	}
	struct ramfsInode *inode = (struct ramfsInode *)entry->inode;
	inode->base.nrofLinks--;
	if (!inode->base.nrofLinks) {
		//delete contents
		deallocPages(inode->fileAddr, inode->nrofPages * PAGE_SIZE);
		//delete inode
		fsDeleteInode(&inode->base);
		kfree(inode);
	}
	//delete direntry
	struct ramfsInode *dir2 = (struct ramfsInode *)dir;
	uintptr_t entryOffset = (uintptr_t)entry - (uintptr_t)dir2->fileAddr;
	void *next = entry + 1;
	memcpy(entry, next, dir2->base.fileSize - entryOffset - sizeof(struct dirEntry));
	dir2->base.fileSize -= sizeof(struct dirEntry);
	if (!(dir2->base.fileSize % PAGE_SIZE)) {
		//dealloc page
		dir2->nrofPages--;
		void *pageAddr = (void *)((uintptr_t)dir2->fileAddr + dir2->nrofPages * PAGE_SIZE);
		deallocPages(dir2, PAGE_SIZE);
	}
	return 0;
}