#include <fs/fs.h>

#include <mm/paging.h>
#include <print.h>

struct rbNode *activeInodes;

struct inode *rootDir;

char testString[] = "Test string!";

int mountRoot(struct inode *rootInode) {
	rootDir = rootInode;
	return 0;
}
/*
static int findSlash(const char *str, size_t len, int pos) {
	for (int i = pos; i < len; i++) {
		if (str[i] == '/') {
			return i;
		}
	}
	return -1;
}

struct inode *getBaseDir(struct inode *cwd, const char *path) {
	char name[256];
	size_t pathLen = strlen(path);
	int curNameStart = 0;
	int curNameEnd;
	struct inode *curDir = cwd;
	while ((curNameEnd = findSlash(path, pathLen, curNameStart)) >= 0) {
		if (curNameEnd == curNameStart) {
			//empty name
			curNameStart++;
			continue;
		}
		int curNameLen = curNameEnd - curNameStart;
		if (curNameLen >= 255) {
			//name too long
			printk("Entry in path too long: %s", path);
			return NULL;
		}

		memcpy(name, &path[curNameStart], curNameLen);
		name[curNameLen] = 0;

		curNameStart = curNameEnd + 1;
	}
}*/

static void listFiles(struct inode *dir) {
	struct cachedDir *cd = dir->cachedData;
	printk("files:\n");
	for (unsigned int i = 0; i < cd->nrofEntries; i++) {
		struct dirEntry *entry = &cd->entries[i];
		char *name;
		if (entry->nameLen > 31) {
			name = entry->name;
		} else {
			name = entry->inlineName;
		}
		printk("->%s\n", name);
	}
}

void fstest(void) {
	struct file newFile;
	int error = fsCreate(&newFile, rootDir, "newfile.txt", ITYPE_FILE);
	fsWrite(&newFile, testString, sizeof(testString));
	listFiles(rootDir);

	struct file newFile2;
	struct dirEntry *entry = dirCacheLookup(rootDir, "newfile.txt");
	if (!entry) {
		printk("Entry not found!\n");
		return;
	}
	fsOpen(entry, &newFile2);
	char buf[64];
	fsRead(&newFile2, buf, 64);
	printk("contents: %s\n", buf);
	
	fsLink(rootDir, newFile2.inode, "newfile2.txt");
	listFiles(rootDir);
	fsUnlink(entry);
	listFiles(rootDir);
}