#include <fs/fs.h>

#include <mm/paging.h>
#include <print.h>

struct rbNode *activeInodes;

//struct inode *rootDir;
struct dirEntry rootDir;

char testString[] = "Test string!";

int fsAddInode(struct inode *node) {
	rbInsert(&activeInodes, &node->rbHeader);
	return 0;
}

int mountRoot(struct inode *rootInode) {
	fsAddInode(rootInode);
	rootDir.inode = rootInode;
	return 0;
}

static void listFiles(struct inode *dir) {
	struct file dfile;
	dir->fOps->open(NULL, &dfile, NULL);
	struct dirEntry entries[16];
	ssize_t read = dfile.fOps->read(&dfile, entries, 16 * sizeof(struct dirEntry));
	for (int i = 0; i < read / sizeof(struct dirEntry); i++) {
		printk("%s\n", entries[i].inlineName);
	}
}

void fstest(void) {
	struct file newFile;
	rootDir.inode->iOps->create(rootDir.inode, "newfile.txt", ITYPE_FILE);
	asm ("xchg bx, bx");
	rootDir.inode->fOps->open(rootDir.inode, &newFile, "newfile.txt");
	newFile.fOps->write(&newFile, testString, sizeof(testString));
	listFiles(rootDir.inode);

	struct file newFile2;
	rootDir.inode->fOps->open(rootDir.inode, &newFile2, "newfile.txt");
	char buf[64];
	newFile2.fOps->read(&newFile2, buf, 64);
	printk("contents: %s\n", buf);
}