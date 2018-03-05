#ifndef INCLUDE_SCHED_PROCESS_H
#define INCLUDE_SCHED_PROCESS_H

/*
Userspace process information
*/

#include <sched/thread.h>
#include <fs/fs.h>
#include <sched/spinlock.h>
#include <mm/paging.h>

#define NROF_INLINE_FDS	8

#define INIT_PID	1

#define MEM_FLAG_WRITE	(1 << 0)
#define MEM_FLAG_EXEC	(1 << 1)
#define MEM_FLAG_SHARED	(1 << 2)

#define PROCFILE_FLAG_USED		SYSOPEN_FLAG_CREATE
#define PROCFILE_FLAG_READ		SYSOPEN_FLAG_READ
#define PROCFILE_FLAG_WRITE		SYSOPEN_FLAG_WRITE
#define PROCFILE_FLAG_CLOEXEC	SYSOPEN_FLAG_CLOEXEC
#define PROCFILE_FLAG_APPEND	SYSOPEN_FLAG_APPEND
#define PROCFILE_FLAG_SHARED	(1 << NROF_SYSOPEN_FLAGS)

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

struct ProcessFile {
	unsigned int flags;
	union {
		struct File file;
		struct File *sharedFile;
	};
};

enum ProcessState {
	PROCSTATE_RUNNING = 0,
	PROCSTATE_WAIT, //Waiting for sleep or waitpid
	PROCSTATE_FINISHED, //zombie process
	PROCSTATE_REMOVE //zombie process after waitpid has been called
};

struct Process {
	pid_t pid;
	pid_t ppid;
	physPage_t addressSpace;
	union {
		char inlineName[32];
		char *name;
	};
	unsigned int nameLen;

	enum ProcessState state;

	struct Process *parent;
	struct Process *children;
	//Owned by parent lock
	struct Process *prevChild;
	struct Process *nextChild;

	struct MemoryEntry *memEntries;
	struct MemoryEntry *brkEntry;
	unsigned int nrofMemEntries;
	spinlock_t memLock;

	struct Inode *cwd; //unused

	unsigned long id;
	void *rootPT;

	spinlock_t lock;

	int exitValue;

	thread_t mainThread;

	struct ProcessFile inlineFDs[NROF_INLINE_FDS];
	struct ProcessFile *fds;
	int nrofFDs; //excluding inlined FDs
	int nrofUsedFDs; //ditto
	spinlock_t fdLock;
};

extern struct Process initProcess;

void linkChild(struct Process *parent, struct Process *child);

int destroyProcessMem(struct Process *proc);

void removeProcess(struct Process *proc);

void exitProcess(struct Process *proc, int exitValue);

#endif