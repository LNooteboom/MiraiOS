#include <fs/fs.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <print.h>

int fsOpen(struct dirEntry *file, struct file *output) {
	if (!file->inode) {
		return -EINVAL;
	}

	if ((file->inode->type & ITYPE_MASK) == ITYPE_DIR) {
		return -EISDIR;
	}
	acquireSpinlock(&file->inode->lock);

	file->inode->refCount++;
	output->path = file;
	output->inode = file->inode;
	//output->fOps = file->inode->fOps;
	output->refCount = 1;
	output->lock = 0;
	output->offset = 0;

	releaseSpinlock(&file->inode->lock);

	return 0;
}

int fsCreate(struct file *output, struct inode *dir, const char *name, uint32_t type) {
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

	struct inode *newInode;
	if (dir->ramfs) {
		newInode = kmalloc(sizeof(struct inode));
		if (!newInode) {
			return -ENOMEM;
		}
	} else {
		//add inode cache entry
		//unimplemented
		printk("Error creating inode: unimplemented");
		return -ENOSYS;
	}

	memset(newInode, 0, sizeof(struct inode));

	newInode->inodeID = dir->superBlock->curInodeID;
	dir->superBlock->curInodeID += 1;
	newInode->superBlock = dir->superBlock;
	newInode->type = type;
	//newInode->dOps = dir->dOps;
	//newInode->fOps = dir->fOps;
	newInode->attr.accessPermissions = 0664;
	newInode->nrofLinks = 1;
	newInode->ramfs = dir->ramfs;

	entry.inode = newInode;
	entry.parent = dir;
	entry.superBlock = dir->superBlock;

	if (!newInode->ramfs) {
		//fsAddInode(newInode);
	}
	struct dirEntry *newEntry = &entry;
	int error = dirCacheAdd(&newEntry, dir);

	releaseSpinlock(&dir->lock);

	if (error) {
		kfree(newInode);
		if (entry.nameLen > 31) {
			kfree(entry.name);
		}
		return error;
	}
	if (output) {
		output->path = newEntry;
		output->inode = newInode;
		//output->fOps = newInode->fOps;
		output->refCount = 1;
		output->lock = 0;
		output->offset = 0;
	}
	return 0;
}
