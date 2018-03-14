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

#define MMAP_FLAG_EXEC		(1 << 0)
#define MMAP_FLAG_WRITE		(1 << 1)
#define MMAP_FLAG_SHARED	(1 << 2) /*Indicates that .shared is valid, has nothing to do with POSIX MAP_SHARED*/
#define MMAP_FLAG_FIXED		(1 << 3)
#define MMAP_FLAG_ANON		(1 << 4)
#define MMAP_FLAG_COW		(1 << 5) /*This flag specifies whether the actual memory mapping is shared or not*/

#define PROCFILE_FLAG_USED		SYSOPEN_FLAG_CREATE
#define PROCFILE_FLAG_READ		SYSOPEN_FLAG_READ
#define PROCFILE_FLAG_WRITE		SYSOPEN_FLAG_WRITE
#define PROCFILE_FLAG_CLOEXEC	SYSOPEN_FLAG_CLOEXEC
#define PROCFILE_FLAG_APPEND	SYSOPEN_FLAG_APPEND
#define PROCFILE_FLAG_SHARED	(1 << NROF_SYSOPEN_FLAGS)

struct SharedMemory {
	unsigned int refCount;
	spinlock_t lock;
	long nrofPages;
	physPage_t phys[1];
};

struct MemoryEntry {
	void *vaddr;
	size_t size;
	unsigned int flags;

	struct MemoryEntry *next;
	struct MemoryEntry *prev;

	struct SharedMemory *shared;
	size_t sharedOffset;
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

	struct MemoryEntry *firstMemEntry;
	struct MemoryEntry *lastMemEntry;
	//struct MemoryEntry *brkEntry;
	spinlock_t memLock;

	struct Inode *cwd;

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

void removeProcess(struct Process *proc);

void exitProcess(struct Process *proc, int exitValue);

void sysExit(int exitValue);

#endif