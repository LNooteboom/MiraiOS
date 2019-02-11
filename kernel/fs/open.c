#include <fs/fs.h>
#include <fs/devfile.h>
#include <errno.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <print.h>
#include <sched/thread.h>
#include <sched/process.h>

static inline void setPerm(struct Inode *inode, uint32_t mode) {
	thread_t curThread = getCurrentThread();
	if (!curThread->process) {
		//kernel thread
		inode->attr.perm = mode;
		return;
	}
	struct ProcessCred *cred = &curThread->process->cred;
	inode->attr.uid = cred->euid;
	inode->attr.gid = cred->egid;
	inode->attr.perm = mode;
}

int fsOpen(struct File *f) {
	int error = 0;
	f->lock = 0;
	f->refCount = 1;

	acquireSpinlock(&f->inode->lock);
	
	if (f->flags & SYSOPEN_FLAG_DIR && !isDir(f->inode)) {
		error = -ENOTDIR;
		goto release;
	}

	if (f->flags & SYSOPEN_FLAG_APPEND) {
		f->flags &= ~SYSOPEN_FLAG_APPEND;
		f->offset = f->inode->fileSize;
	} else {
		f->offset = 0;
	}
	f->inode->refCount++;

	release:
	releaseSpinlock(&f->inode->lock);
	return error;
}

void fsClose(struct File *file) {
	acquireSpinlock(&file->lock);
	if (!file->inode) {
		releaseSpinlock(&file->lock);
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

int fsCreate(struct File *f, struct Inode *dir, const char *name, uint32_t type) {
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
	newInode->type = type & 0xFF;
	newInode->nrofLinks = 1;
	newInode->ramfs = dir->ramfs;

	setPerm(newInode, type >> CREATE_PERM_SHIFT);


	if (isDir(newInode)) {
		dirCacheInit(newInode, dir);
	}

	entry.inode = newInode;

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
	if (f) {
		f->inode = newInode;
		fsOpen(f);
	}
	return 0;
}