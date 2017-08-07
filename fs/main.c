#include <fs/fs.h>

#include <mm/paging.h>

struct rbNode *activeInodes;

//struct inode *rootDir;
struct dirEntry rootDir;

int fsAddInode(struct inode *node) {
	rbInsert(&activeInodes, &node->rbHeader);
	return 0;
}

int mountRoot(struct inode *rootInode) {
	fsAddInode(rootInode);
	rootDir.inode = rootInode;
	return 0;
}

void fstest(void) {
	struct file root;
	rootDir.inode->fOps->open(&root, &rootDir);
	struct dirEntry entries[8];
	root.fOps->read(&root, entries, 8 * sizeof(struct dirEntry));
	printk("root dir entry 1: %s\n\n", entries[0].inlineName);
	struct file testf;
	entries[0].inode->fOps->open(&testf, &entries[0]);
	char *contents = allocKPages(0x1000, PAGE_FLAG_INUSE | PAGE_FLAG_WRITE);
	testf.fOps->read(&testf, contents, 0x1000);
	puts(contents);
}