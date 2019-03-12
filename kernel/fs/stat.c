#include <fs/fs.h>
#include <uapi/stat.h>
#include <errno.h>
#include <sched/thread.h>
#include <sched/process.h>
#include <userspace.h>

static int doStat(struct Inode *inode, struct stat *statBuf) {
	statBuf->st_dev = inode->superBlock->fsID;
	statBuf->st_ino = inode->inodeID;
	statBuf->st_mode = inode->type;
	statBuf->st_nlink = inode->nrofLinks;
	statBuf->st_uid = inode->attr.uid;
	statBuf->st_gid = inode->attr.gid;
	statBuf->st_rdev = 0;
	statBuf->st_size = inode->fileSize;
	return 0;
}

int sysStat(const char *fileName, struct stat *statBuf) {
	struct stat ret;
	struct Inode *inode;

	int error = validateUserString(fileName);
	if (error) return error;
	error = validateUserPointer(statBuf, sizeof(*statBuf));
	if (error) return error;

	struct Process *proc = getCurrentThread()->process;
	inode = getInodeFromPath(proc->cwd, fileName);
	if (!inode) return -ENOENT;

	error = doStat(inode, statBuf);

	return error;
}

int sysLstat(const char *fileName, struct stat *statBuf) {
	//TODO
	return sysStat(fileName, statBuf);
}

int sysFstatat(int dirfd, const char *fileName, struct stat *statBuf, int flags) {
	//TODO
	return -ENOSYS;
}