#include <fs/fs.h>

#include <fs/devfile.h>
#include <mm/paging.h>
#include <errno.h>
#include <print.h>
#include <mm/memset.h>
#include <mm/heap.h>
#include <sched/process.h>


struct RbNode *activeInodes;

struct Inode *rootDir;

int mountRoot(struct Inode *rootInode) {
	rootDir = rootInode;
	return 0;
}

static int findSlash(const char *str, size_t len, int pos) {
	for (unsigned int i = pos; i < len; i++) {
		if (str[i] == '/') {
			return i;
		}
	}
	return -1;
}

static struct DirEntry *getDirEntryFromPath(struct Inode *cwd, const char *path, int *fileNameIndex) {
	char name[256];
	size_t pathLen = strlen(path);
	int curNameStart;
	int curNameEnd = -1; //points to next slash
	struct Inode *curDir = (cwd)? cwd : rootDir;

	if (path[0] == '/') {
		curDir = rootDir;
		curNameEnd = 0;
	}

	while (true) {
		curNameStart = curNameEnd + 1;
		int slash = findSlash(path, pathLen, curNameStart);
		if (slash < 0 || slash == (int)pathLen - 1) {
			curNameEnd = (slash < 0)? (int)pathLen : slash;
			memcpy(name, &path[curNameStart], curNameEnd - curNameStart);
			name[curNameEnd - curNameStart] = 0;
			if (fileNameIndex) {
				*fileNameIndex = curNameStart;
				return (struct DirEntry *)curDir;
			} else {
				acquireSpinlock(&curDir->lock);
				struct DirEntry *file = dirCacheLookup(curDir, name);
				return file; //SPINLOCK MUST BE RELEASED BY THE CALLER
			}
		}
		curNameEnd = slash;

		int len = curNameEnd - curNameStart;
		if (!len) {
			continue; //skip double slash
		}
		memcpy(name, &path[curNameStart], len);
		name[len] = 0;

		acquireSpinlock(&curDir->lock);
		struct DirEntry *entry = dirCacheLookup(curDir, name);
		if (!entry || !isDir(entry->inode)) {
			releaseSpinlock(&curDir->lock);
			return NULL;
		}
		struct Inode *newCurDir = entry->inode;
		releaseSpinlock(&curDir->lock);
		curDir = newCurDir;
	}
}

struct Inode *getInodeFromPath(struct Inode *cwd, const char *path) {
	if (!path[0]) return NULL;
	if (path[0] == '/' && !path[1]) return rootDir;
	struct DirEntry *entry = getDirEntryFromPath(cwd, path, NULL);
	if (!entry) {
		return NULL;
	}
	struct Inode *ret = entry->inode;
	releaseSpinlock(&entry->parent->lock);
	return ret;
}

struct Inode *getBaseDirFromPath(struct Inode *cwd, int *fileNameIndex, const char *path) {
	if (!path[0]) return NULL;
	if (path[0] == '/' && !path[1]) return rootDir;
	return (struct Inode *)getDirEntryFromPath(cwd, path, fileNameIndex);
}

int fsGetDent(struct Inode *dir, struct GetDent *buf, unsigned int index) {
	acquireSpinlock(&dir->lock);

	int ret;
	if (dir->cachedData) {
		ret = dirCacheGet(dir, buf, index);
	} else {
		//load from FS
		ret = -ENOSYS;
	}

	releaseSpinlock(&dir->lock);
	return ret;
}

int fsCloseOnExec(void) {
	struct Process *proc = getCurrentThread()->process;
	for (int i = 0; i < NROF_INLINE_FDS; i++) {
		if (proc->inlineFDs[i].flags & PROCFILE_FLAG_USED && proc->inlineFDs[i].flags & PROCFILE_FLAG_CLOEXEC) {
			sysClose(i);
		}
	}
	for (int i = 0; i < proc->nrofFDs; i++) {
		if (proc->fds[i].flags & PROCFILE_FLAG_USED && proc->fds[i].flags & PROCFILE_FLAG_CLOEXEC) {
			sysClose(i);
		}
	}
	return 0;
}
