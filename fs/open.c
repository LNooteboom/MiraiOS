#include <fs/fs.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <print.h>

int fsOpen(struct Inode *inode, struct File *output) {

	if ((inode->type & ITYPE_MASK) == ITYPE_DIR) {
		return -EISDIR;
	}
	acquireSpinlock(&inode->lock);

	inode->refCount++;
	output->inode = inode;
	output->lock = 0;
	output->offset = 0;

	releaseSpinlock(&inode->lock);

	return 0;
}

int fsCreate(struct File *output, struct Inode *dir, const char *name, uint32_t type) {
	struct DirEntry entry;

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

	struct Inode *newInode;
	if (dir->ramfs) {
		newInode = kmalloc(sizeof(struct Inode));
		if (!newInode) {
			return -ENOMEM;
		}
	} else {
		//add inode cache entry
		//unimplemented
		printk("Error creating inode: unimplemented");
		return -ENOSYS;
	}

	memset(newInode, 0, sizeof(struct Inode));

	newInode->inodeID = dir->superBlock->curInodeID;
	dir->superBlock->curInodeID += 1;
	newInode->superBlock = dir->superBlock;
	newInode->type = type;
	newInode->attr.accessPermissions = 0664;
	newInode->nrofLinks = 1;
	newInode->ramfs = dir->ramfs;

	if ((type & ITYPE_MASK) == ITYPE_DIR) {
		dirCacheInit(newInode, dir);
	}

	entry.inode = newInode;

	if (!newInode->ramfs) {
		//fsAddInode(newInode);
	}
	struct DirEntry *newEntry = &entry;
	int error = dirCacheAdd(&newEntry, dir);

	releaseSpinlock(&dir->lock);

	if (error) {
		kfree(newInode);
		if (entry.nameLen > 31) {
			kfree(entry.name);
		}
		return error;
	}
	if (output && (newInode->type & ITYPE_MASK) == ITYPE_FILE) {
		output->inode = newInode;
		output->lock = 0;
		output->offset = 0;
	}
	return 0;
}

int fsGetDirEntry(struct Inode *dir, unsigned int index, struct UserDentry *dentry) {
	acquireSpinlock(&dir->lock);

	if ((dir->type & ITYPE_MASK) != ITYPE_DIR) {
		releaseSpinlock(&dir->lock);
		return -EINVAL;
	}
	struct CachedDir *cd = dir->cachedData;
	if (!cd) {
		//load from fs
		return -ENOSYS;
	}
	if (index >= cd->nrofEntries) {
		return -EINVAL;
	}

	char *name = cd->entries[index].inlineName;
	size_t len = cd->entries[index].nameLen;
	if (len > 31) {
		name = cd->entries[index].name;
	}
	memcpy(&dentry->name, name, len);
	dentry->name[len] = 0;
	dentry->inode = cd->entries[index].inode;

	releaseSpinlock(&dir->lock);
	return 0;
}