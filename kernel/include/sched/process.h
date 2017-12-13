#ifndef INCLUDE_SCHED_PROCESS_H
#define INCLUDE_SCHED_PROCESS_H

#include <sched/thread.h>
#include <fs/fs.h>
#include <sched/spinlock.h>
#include <mm/paging.h>

#define NROF_INLINE_FDS	8

#define INIT_PID	1

#define MEM_FLAG_WRITE	(1 << 0)
#define MEM_FLAG_EXEC	(1 << 1)
#define MEM_FLAG_SHARED	(1 << 2)

struct ProcessMemory;

struct SharedMemory {
	void *vaddr;
	size_t size;
	unsigned int flags;
	unsigned int refCount;
	unsigned long nrofPhysPages;
	uintptr_t alignedVaddr;

	spinlock_t lock;

	physPage_t phys[1]; //extendable
};

struct MemoryEntry {
	void *vaddr;
	size_t size;
	unsigned int flags;
	struct SharedMemory *shared;
};

struct ProcessMemory {
	unsigned int nrofEntries;
	struct MemoryEntry *entries;
};

struct Process {
	uint64_t pid;
	physPage_t addressSpace;
	union {
		char inlineName[32];
		char *name;
	};
	unsigned int nameLen;

	struct ProcessMemory pmem;

	char *cwd;

	unsigned long id;
	void *rootPT;

	spinlock_t lock;
	int exitValue;

	thread_t mainThread;

	struct File inlineFDs[NROF_INLINE_FDS];
	struct File *fds;
};

#endif