#include <fs/fs.h>
#include <fs/fd.h>
#include <fs/pipe.h>
#include <mm/memset.h>
#include <mm/heap.h>
#include <errno.h>
#include <userspace.h>

int pipeWrite(struct Pipe *pipe, const void *buffer, size_t size) {
	const char *cbuf = (const char *)buffer;
	while (size) {
		semWait(&pipe->bytesFree);
		pipe->buf[pipe->wIndex] = *cbuf;
		semSignal(&pipe->bytesUsed);

		pipe->wIndex = (pipe->wIndex + 1) % PIPE_BUF;
		cbuf++;
		size--;
	}
	return 0;
}

ssize_t pipeRead(struct Pipe *pipe, void *buffer, size_t size) {
	char *cbuf = (char *)buffer;
	ssize_t ret = 0;
	while (ret != (ssize_t)size) {
		semWait(&pipe->bytesUsed);
		*cbuf = pipe->buf[pipe->rIndex];
		semSignal(&pipe->bytesFree);

		pipe->rIndex = (pipe->rIndex + 1) % PIPE_BUF;
		ret++;
		cbuf++;
	}
	return ret;
}

int sysPipe(int fd[2], int flags) {
	int error = validateUserPointer(fd, sizeof(*fd) * 2);
	if (error) return error;
	int readFD;
	int writeFD;

	struct Process *proc = getCurrentThread()->process;
	acquireSpinlock(&proc->fdLock);

	struct ProcessFile *rf;
	readFD = allocateFD(proc, &rf);
	if (readFD < 0) {
		error = readFD;
		releaseSpinlock(&proc->fdLock);
		goto release;
	}
	struct ProcessFile *wf;
	writeFD = allocateFD(proc, &wf);
	if (writeFD < 0) {
		error = readFD;
		releaseSpinlock(&proc->fdLock);
		goto deallocReadFD;
	}

	void *buf = allocKPages(PIPE_BUF, PAGE_FLAG_WRITE);
	if (!buf) {
		error = -ENOMEM;
		releaseSpinlock(&proc->fdLock);
		goto deallocWriteFD;
	}

	struct Pipe *pipe = kmalloc(sizeof(*pipe));
	if (!pipe) {
		error = -ENOMEM;
		releaseSpinlock(&proc->fdLock);
		goto deallocBuf;
	}
	memset(pipe, 0, sizeof(*pipe));
	pipe->buf = buf;
	pipe->refCount = 2;
	semInit(&pipe->bytesFree, PIPE_BUF);
	semInit(&pipe->bytesUsed, 0);

	if (flags & SYSOPEN_FLAG_CLOEXEC) {
		rf->flags |= PROCFILE_FLAG_CLOEXEC;
	}
	rf->flags |= PROCFILE_FLAG_PIPE;
	wf->flags = rf->flags | PROCFILE_FLAG_PIPE_WRITE;
	rf->pipe = pipe;
	wf->pipe = pipe;

	fd[0] = readFD;
	fd[1] = writeFD;

	goto release;

	deallocBuf:
	deallocPages(buf, PIPE_BUF);
	deallocWriteFD:
	sysClose(writeFD);
	deallocReadFD:
	sysClose(readFD);
	return error;

	release:
	releaseSpinlock(&proc->fdLock);
	return error;
}

void copyFD(struct ProcessFile *pf, struct ProcessFile *newPF, struct File *shared) {
	if ( !(pf->flags & PROCFILE_FLAG_USED)) {
		return;
	}
	if (pf->flags & PROCFILE_FLAG_PIPE) {
		pf->pipe->refCount++;
		newPF->flags = pf->flags;
		newPF->pipe = pf->pipe;
		return;
	}
	if ( !(pf->flags & PROCFILE_FLAG_SHARED)) {
		memset(shared, 0, sizeof(struct File));
		shared->inode = pf->file.inode;
		shared->refCount = 1;
		pf->flags |= PROCFILE_FLAG_SHARED;
		pf->sharedFile = shared;
	}
	newPF->flags = pf->flags;
	newPF->sharedFile = pf->sharedFile;
	pf->sharedFile->refCount++;
	return;
}

int sysDup(int oldFD, int newFD, int flags) {
	int error = 0;
	struct Process *proc = getCurrentThread()->process;
	acquireSpinlock(&proc->fdLock);

	struct ProcessFile *oldPF;
	if (oldFD < NROF_INLINE_FDS) {
		oldPF = &proc->inlineFDs[oldFD];
	} else {
		if (oldFD - NROF_INLINE_FDS >= proc->nrofFDs) {
			error = -EBADF;
			goto release;
		}
		oldPF = &proc->fds[oldFD - NROF_INLINE_FDS];
	}
	if ( !(oldFD & PROCFILE_FLAG_USED)) {
		error = -EBADF;
		goto release;
	}
	struct File *shared = NULL;
	if ( !(oldPF->flags & PROCFILE_FLAG_PIPE || oldPF->flags & PROCFILE_FLAG_SHARED)) {
		shared = kmalloc(sizeof(*shared));
		if (!shared) {
			error = -ENOMEM;
			goto release;
		}
	}

	struct ProcessFile *newPF;
	if (newFD < 0) {
		newFD = allocateFD(proc, &newPF);
	} else {
		if (newFD >= MAX_FDS) {
			error = -EBADF;
			goto deallocShared;
		}
		if (newFD - NROF_INLINE_FDS >= proc->nrofFDs) {
			struct ProcessFile *fds = krealloc(proc->fds, (newFD - NROF_INLINE_FDS + 1) * sizeof(*fds));
			if (!fds) {
				error = -ENOMEM;
				goto deallocShared;
			}
			memset(fds + proc->nrofFDs, 0, newFD - NROF_INLINE_FDS + 1 - proc->nrofFDs);
			proc->fds = fds;
			proc->nrofFDs = newFD - NROF_INLINE_FDS + 1;
			proc->nrofUsedFDs += newFD - NROF_INLINE_FDS;
			newPF = &fds[newFD - NROF_INLINE_FDS];
		} else {
			releaseSpinlock(&proc->fdLock);
			sysClose(newFD);
			acquireSpinlock(&proc->fdLock);
		}
	}

	copyFD(oldPF, newPF, shared);
	if (flags & SYSOPEN_FLAG_CLOEXEC) {
		newPF->flags |= PROCFILE_FLAG_CLOEXEC;
	} else {
		newPF->flags &= ~PROCFILE_FLAG_CLOEXEC;
	}

	releaseSpinlock(&proc->fdLock);
	return newFD;
	
	deallocShared:
	if (shared) {
		kfree(shared);
	}
	release:
	releaseSpinlock(&proc->fdLock);
	return error;
}