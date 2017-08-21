#include <fs/fs.h>
#include <fs/direntry.h>
#include <errno.h>
#include <mm/memset.h>
#include <mm/heap.h>
#include <mm/paging.h>
#include <print.h>

int fsLink(struct inode *dir, struct inode *inode, const char *name) {
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

	acquireSpinlock(&dir->lock);

	entry.nameLen = nameLen;
	entry.inode = inode;
	entry.parent = dir;
	entry.superBlock = dir->superBlock;
	inode->nrofLinks += 1;
	struct dirEntry *pEntry = &entry;
	dirCacheAdd(&pEntry, dir);

	releaseSpinlock(&dir->lock);
	
	return 0;
}

int fsUnlink(struct dirEntry *entry) {
	if (!entry) {
		return -ENOENT;
	}

	struct inode *inode = entry->inode;
	if (!inode->ramfs) { //temp
		printk("Error deleting inode: unimplemented");
		return -ENOSYS;
	}
	
	acquireSpinlock(&inode->lock);

	inode->nrofLinks--;
	if (!inode->nrofLinks) {
		releaseSpinlock(&inode->lock);
		if (inode->ramfs) {
			//delete cache
			if (inode->cachedData) {
				if ((inode->type & ITYPE_MASK) == ITYPE_DIR) {
					dirCacheDelete(inode);
				} else if (!(inode->ramfs & RAMFS_INITRD)) {
					//file
					deallocPages(inode->cachedData, inode->cachedDataSize);
				}
			}
			//delete inode
			kfree(inode);
		} else {
			//call fs drivers
		}
	} else {
		releaseSpinlock(&inode->lock);
	}
	//delete direntry
	dirCacheRemove(entry);
	return 0;
}