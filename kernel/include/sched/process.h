#ifndef INCLUDE_SCHED_PROCESS_H
#define INCLUDE_SCHED_PROCESS_H

#include <sched/thread.h>
#include <fs/fs.h>

#define NROF_INLINE_FDS	8

#define INIT_PID	1

struct Process {
	unsigned long pid;
	void *addressSpace;
	union {
		char inlineName[32];
		char *name;
	};
	unsigned int nameLen;

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