#include <fs/fs.h>
#include <fs/devfile.h>
#include <sched/process.h>
#include <mm/heap.h>
#include <errno.h>
#include <syscall.h>
#include <modules.h>
#include <stdarg.h>

static struct File *getFileFromFD(struct Process *proc, int fd) {
	struct ProcessFile *pf;
	struct File *f;
	if (fd < NROF_INLINE_FDS) {
		pf = &proc->inlineFDs[fd];
	} else if (fd - NROF_INLINE_FDS < proc->nrofFDs && proc->fds[fd].flags & PROCFILE_FLAG_USED) {
		pf = &proc->fds[fd - NROF_INLINE_FDS];
	} else {
		return NULL; //EBADF
	}
	f = (pf->flags & PROCFILE_FLAG_SHARED)? pf->sharedFile : &pf->file;
	if (!f->inode) {
		return NULL; //EBADF
	}

	return f;
}

static int allocateFD(struct Process *proc) {
	int fileno = 0;
	struct ProcessFile *file = NULL;

	for (unsigned int i = 0; i < NROF_INLINE_FDS; i++) {
		if ( !(proc->inlineFDs[i].flags & PROCFILE_FLAG_USED) ) {
			file = &proc->inlineFDs[i];
			fileno = i;
			break;
		}
	}
	if (!file && !proc->fds) {
		proc->fds = kmalloc(sizeof(struct ProcessFile));
		if (!proc->fds) {
			return -ENOMEM;
		}
		proc->nrofFDs = 1;
	} else if (!file) {
		//search for free entry
		for (int i = 0; i < proc->nrofFDs; i++) {
			if ( !(proc->fds[i].flags & PROCFILE_FLAG_USED)) {
				file = &proc->fds[i];
				fileno = i + NROF_INLINE_FDS;
				break;
			}
		}
		if (!file) {
			proc->nrofFDs++;
			file = krealloc(proc->fds, proc->nrofFDs * sizeof(struct ProcessFile));
			if (!file) {
				return -ENOMEM;
			}
			proc->fds = file;
			file = &proc->fds[proc->nrofFDs - 1];
			fileno = NROF_INLINE_FDS;
		}
	}
	file->flags = PROCFILE_FLAG_USED;
	return fileno;
}

int sysWrite(int fd, void *buffer, size_t size) {
	int error = validateUserPointer(buffer, size);
	if (error) {
		return error;
	}

	struct File *f = getFileFromFD(getCurrentThread()->process, fd);
	if (!f) {
		return -EBADF;
	}
	error = fsWrite(f, buffer, size);
	return error;
}

int sysRead(int fd, void *buffer, size_t size) {
	int error = validateUserPointer(buffer, size);
	if (error) {
		return error;
	}

	struct File *f = getFileFromFD(getCurrentThread()->process, fd);
	if (!f) {
		return -EBADF;
	}
	error = fsRead(f, buffer, size);
	return error;
}

int sysIoctl(int fd, unsigned long request, ...) {
	va_list args;
	va_start(args, request);

	struct File *f = getFileFromFD(getCurrentThread()->process, fd);
	if (!f) {
		return -EBADF;
	}

	acquireSpinlock(&f->lock);
	acquireSpinlock(&f->inode->lock);

	struct DevFileOps *ops = f->inode->ops;
	int ret = -ENOSYS;
	if ((f->inode->type & ITYPE_MASK) == ITYPE_CHAR && ops && ops->ioctl) {
		ret = ops->ioctl(f, request, args);
	}

	releaseSpinlock(&f->inode->lock);
	releaseSpinlock(&f->lock);

	va_end(args);

	return ret;
}

int sysOpen(const char *fileName, unsigned int flags) {
	int error = validateUserString(fileName);
	if (error) return error;

	struct Process *proc = getCurrentThread()->process;
	acquireSpinlock(&proc->lock);
	int fileno = allocateFD(proc);
	releaseSpinlock(&proc->lock);
	if (fileno < 0) {
		return fileno;
	}

	struct ProcessFile *file = (fileno < NROF_INLINE_FDS)? &proc->inlineFDs[fileno] : &proc->fds[fileno - NROF_INLINE_FDS];
	struct Inode *inode = getInodeFromPath(proc->cwd, fileName);
	if (!inode && flags & SYSOPEN_FLAG_CREATE) {
		//create file
		int fileNameIndex;
		struct Inode *dir = getBaseDirFromPath(proc->cwd, &fileNameIndex, fileName);
		if (!dir) {
			return -ENOENT; //dir does not exist
		}
		error = fsCreate(&file->file, dir, fileName, ITYPE_FILE);
		if (error) {
			return error;
		}
	} else if (!inode) {
		return -ENOENT;
	}
	error = fsOpen(inode, &file->file);
	if (error) {
		return error;
	}
	file->flags = (flags | PROCFILE_FLAG_USED) & ((1 << NROF_SYSOPEN_FLAGS) - 1);

	return fileno;
}

int sysClose(int fd) {
	struct Process *proc = getCurrentThread()->process;
	struct ProcessFile *pf = (fd < NROF_INLINE_FDS)? &proc->inlineFDs[fd] : &proc->fds[fd - NROF_INLINE_FDS];
	if ( !(pf->flags & PROCFILE_FLAG_USED)) {
		return -EBADF;
	}
	if (pf->flags & PROCFILE_FLAG_SHARED) {
		//todo
		return -ENOSYS;
	}
	acquireSpinlock(&proc->lock);
	fsClose(&pf->file);
	pf->flags = 0;
	if (fd >= NROF_INLINE_FDS) {
		if ( !(--proc->nrofUsedFDs)) {
			kfree(proc->fds);
		}
	}
	releaseSpinlock(&proc->lock);
	return 0;
}

int sysGetDent(int fd, struct GetDent *buf) {
	int ret = validateUserPointer(buf, sizeof(struct GetDent));
	if (ret) return ret;

	struct File *f = getFileFromFD(getCurrentThread()->process, fd);
	if (!f) return -EBADF;
	acquireSpinlock(&f->lock);
	ret = fsGetDent(f->inode, buf, f->offset);
	f->offset += 1;
	releaseSpinlock(&f->lock);
	return ret;
}

int registerFSSyscalls(void) {
	registerSyscall(0, sysRead);
	registerSyscall(1, sysWrite);
	registerSyscall(2, (void *)sysIoctl);
	registerSyscall(3, sysOpen);
	registerSyscall(4, sysClose);
	registerSyscall(5, sysGetDent);
	return 0;
}
MODULE_INIT(registerFSSyscalls);