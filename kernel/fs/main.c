#include <fs/fs.h>

#include <mm/paging.h>
#include <errno.h>
#include <print.h>
#include <mm/memset.h>

struct RbNode *activeInodes;

struct Inode *rootDir;

char testString[] = "Test string!";

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

	while (true) {
		int slash = findSlash(path, pathLen, curNameEnd + 1);
		if (slash < 0 || slash == (int)pathLen - 1) {
			if (fileNameIndex) {
				*fileNameIndex = curNameEnd + 1;
				return (struct DirEntry *)curDir;
			} else {
				acquireSpinlock(&curDir->lock);
				struct DirEntry *file = dirCacheLookup(curDir, &path[curNameEnd + 1]);
				return file; //SPINLOCK MUST BE RELEASED BY THE CALLER
			}
		} else if (slash == 0) {
			curDir = rootDir; //path starts with a slash, use rootdir
			curNameEnd = 0;
			continue;
		}
		curNameStart = curNameEnd + 1;
		curNameEnd = slash;

		int len = curNameEnd - curNameStart;
		if (!len) {
			continue; //skip double slash
		}
		memcpy(name, &path[curNameStart], len);
		name[len] = 0;

		acquireSpinlock(&curDir->lock);
		struct DirEntry *entry = dirCacheLookup(curDir, name);
		if (!entry || (entry->inode->type & ITYPE_MASK) != ITYPE_DIR) {
			releaseSpinlock(&curDir->lock);
			return NULL;
		}
		struct Inode *newCurDir = entry->inode;
		releaseSpinlock(&curDir->lock);
		curDir = newCurDir;
	}
}

struct Inode *getInodeFromPath(struct Inode *cwd, const char *path) {
	struct DirEntry *entry = getDirEntryFromPath(cwd, path, NULL);
	if (!entry) {
		return NULL;
	}
	struct Inode *ret = entry->inode;
	releaseSpinlock(&entry->parent->lock);
	return ret;
}

struct Inode *getBaseDirFromPath(struct Inode *cwd, int *fileNameIndex, const char *path) {
	return (struct Inode *)getDirEntryFromPath(cwd, path, fileNameIndex);
}

int fsGetDents(struct Inode *dir, struct GetDents *buf, unsigned int nrofEntries) {
	acquireSpinlock(&dir->lock);

	int ret = dirCacheList(dir, buf, nrofEntries);

	releaseSpinlock(&dir->lock);
	return ret;
}