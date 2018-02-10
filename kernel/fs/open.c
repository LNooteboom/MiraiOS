#include <fs/fs.h>
#include <fs/devfile.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <print.h>

int fsOpen(struct Inode *inode, struct File *output) {

	/*if ((inode->type & ITYPE_MASK) == ITYPE_DIR) {
		return -EISDIR;
	}*/
	acquireSpinlock(&inode->lock);

	inode->refCount++;
	memset(output, 0, sizeof(struct File));
	output->inode = inode;

	releaseSpinlock(&inode->lock);

	return 0;
}

void fsClose(struct File *file) {
	acquireSpinlock(&file->lock);
	if (!file->inode) {
		return; //BADF
	}
	acquireSpinlock(&file->inode->lock);

	struct DevFileOps *ops = file->inode->ops;
	if ((file->inode->type & ITYPE_MASK) == ITYPE_CHAR && ops && ops->close) {
		ops->close(file);
	}
	file->inode->refCount--;

	releaseSpinlock(&file->inode->lock);
	file->inode = NULL;
	releaseSpinlock(&file->lock);
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