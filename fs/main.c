#include <fs/fs.h>

struct rbNode *activeInodes;

struct inode *rootDir;

int fsAddInode(struct inode *node) {
	rbInsert(&activeInodes, &node->rbHeader);
	return 0;
}

int mountRoot(struct inode *rootInode) {
	fsAddInode(rootInode);
	rootDir = rootInode;
	return 0;
}