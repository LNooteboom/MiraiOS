#ifndef INCLUDE_FS_PIPE_H
#define INCLUDE_FS_PIPE_H

#include <stdatomic.h>
#include <sched/lock.h>
#include <sched/spinlock.h>
#include <uapi/fcntl.h>

struct Pipe {
	semaphore_t bytesFree;
	semaphore_t bytesUsed;

	spinlock_t lock;
	int rIndex;
	int wIndex;

	char *buf;
	atomic_uint refCount;
};

int pipeWrite(struct Pipe *pipe, const void *buffer, size_t size);

ssize_t pipeRead(struct Pipe *pipe, void *buffer, size_t size);

void copyFD(struct ProcessFile *pf, struct ProcessFile *newPF, struct File *shared);

#endif