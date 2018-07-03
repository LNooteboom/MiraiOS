#include <fs/fs.h>
#include <fs/direntry.h>
#include <fs/link.h>
#include <sched/spinlock.h>
#include <errno.h>
#include <mm/memset.h>
#include <mm/heap.h>

int fsRename(struct Inode *newDir, const char *newName, struct Inode *oldDir, const char *oldName, int flags) {
	struct Inode *inode;
	int error;

	acquireSpinlock(&oldDir->lock);

	struct DirEntry *oldEntry = dirCacheLookup(oldDir, oldName);
	if (!oldEntry) {
		error = -ENOENT;
		goto releaseOldDir;
	}
	inode = oldEntry->inode;
	if (oldDir == newDir) {
		//change name only, no need to move between dirs
		if (oldEntry->nameLen >= INLINENAME_MAX) {
			kfree(oldEntry->name);
		}

		int nameLen = strlen(newName);
		if (nameLen < INLINENAME_MAX) {
			memcpy(&oldEntry->inlineName, newName, nameLen);
		} else {
			oldEntry->name = kmalloc(nameLen);
			if (!oldEntry->name) {
				error = -ENOMEM;
				goto releaseOldDir;
			}
			memcpy(&oldEntry->name, newName, nameLen);
		}
		oldEntry->nameLen = nameLen;
		error = 0;
		goto releaseOldDir;
	}
	
	releaseSpinlock(&oldDir->lock);
	
	acquireSpinlock(&newDir->lock);

	struct DirEntry *entry = dirCacheLookup(newDir, newName);
	if (entry) {
		if (flags & RENAME_NOREPLACE) {
			error = -EEXIST;
			goto releaseNewDir;
		}
		//already exists
		if (isDir(inode) && !isDir(entry->inode)) {
			//dir->file
			error = -ENOTDIR;
			goto releaseNewDir;
		} else if (isDir(entry->inode)) {
			//file->dir
			error = -EISDIR;
			goto releaseNewDir;
		}
		error = unlinkInode(entry->inode);
		if (error) goto releaseNewDir;
		entry->inode = inode;
		
	} else {
		int nameLen = strlen(newName);
		struct DirEntry newEntry = {
			.inode = inode,
			.nameLen = nameLen
		};
		
		if (nameLen < INLINENAME_MAX) {
			memcpy(&newEntry.inlineName, newName, nameLen);
		} else {
			newEntry.name = kmalloc(nameLen);
			if (!newEntry.name) {
				error = -ENOMEM;
				goto releaseNewDir;
			}
			memcpy(&newEntry.name, newName, nameLen);
		}

		entry = &newEntry;
		error = dirCacheAdd(&entry, newDir);
	}

	acquireSpinlock(&oldDir->lock);
	oldEntry = dirCacheLookup(oldDir, oldName);
	dirCacheRemove(oldEntry);
	releaseSpinlock(&oldDir->lock);

	releaseNewDir:
	releaseSpinlock(&newDir->lock);
	goto out;

	releaseOldDir:
	releaseSpinlock(&oldDir->lock);
	out:
	return error;
}