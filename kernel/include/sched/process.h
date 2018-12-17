#ifndef INCLUDE_SCHED_PROCESS_H
#define INCLUDE_SCHED_PROCESS_H

/*
Userspace processes
*/

#include <stdatomic.h>
#include <sched/thread.h>
#include <fs/fs.h>
#include <sched/spinlock.h>
#include <mm/paging.h>
#include <uapi/mmap.h>
#include <sched/signal.h>

#define PROC_HT_SIZE	64

#define NROF_INLINE_FDS	8
#define MAX_FDS			64

#define INIT_PID	1

#define MMAP_FLAG_COW		(1 << 5) /*This flag specifies whether the actual memory mapping is shared or not*/

#define PROCFILE_FLAG_USED			0x01
#define PROCFILE_FLAG_CLOEXEC		0x02
#define PROCFILE_FLAG_SHARED		0x04
#define PROCFILE_FLAG_PIPE			0x08
#define PROCFILE_FLAG_PIPE_WRITE	0x10 //Write end of the pipe if set, read end if clear

struct SharedMemory {
	atomic_uint refCount;
	//spinlock_t lock;
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
		struct Pipe *pipe;
	};
};

enum ProcessState {
	PROCSTATE_RUNNING = 0,
	PROCSTATE_WAIT, //Waiting for sleep or waitpid
	PROCSTATE_FINISHED //zombie process
};

struct Session;
struct PGroup;

struct Process {
	pid_t pid;
	pid_t ppid;
	physPage_t addressSpace; //address of root page table
	enum ProcessState state;
	unsigned int nameLen;

	union {
		char inlineName[32];
		char *name;
	};
	
	spinlock_t lock;

	//Hash table list next & prev
	struct Process *htNext;
	struct Process *htPrev;

	struct Process *parent;
	struct Process *children;
	//Owned by parent lock
	struct Process *prevChild;
	struct Process *nextChild;

	//mmap entries
	struct MemoryEntry *firstMemEntry;
	struct MemoryEntry *lastMemEntry;
	//struct MemoryEntry *brkEntry;
	spinlock_t memLock;

	//group info
	pid_t pgid;
	pid_t sid;
	struct PGroup *group;
	struct Process *grpNext;
	struct Process *grpPrev;

	//user info
	uid_t ruid;
	uid_t euid;
	uid_t suid;
	gid_t rgid;
	gid_t egid;
	gid_t sgid;
	unsigned int nrofGroups;
	gid_t groups[NGROUPS_MAX];

	//signal info & handling
	bool sigResched;
	int sigNrPending;
	sigset_t sigMask;
	sigset_t sigPending;
	struct PendingSignal *sigFirst;
	struct PendingSignal *sigLast;
	struct SigRegs *sigRegStack;
	
	struct sigaction sigAct[NROF_SIGNALS];

	void *sigAltStack;
	size_t sigAltStackSize;

	siginfo_t exitInfo;

	thread_t mainThread;

	//file info
	struct Inode *cwd;
	struct ProcessFile inlineFDs[NROF_INLINE_FDS];
	struct ProcessFile *fds;
	int nrofFDs; //excluding inlined FDs
	int nrofUsedFDs; //ditto
	spinlock_t fdLock;
};

extern struct Process initProcess;

void linkChild(struct Process *parent, struct Process *child);

void removeProcess(struct Process *proc);

void exitProcess(struct Process *proc);

void signalExit(void);

void procHTAdd(struct Process *proc);
void procHTDel(struct Process *proc);

struct Process *getProcFromPid(pid_t pid);

#endif