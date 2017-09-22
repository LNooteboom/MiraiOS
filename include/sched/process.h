#ifndef INCLUDE_SCHED_PROCESS_H
#define INCLUDE_SCHED_PROCESS_H

#include <sched/thread.h>

struct process {
	union {
		char inlineName[32];
		char *name;
	};
	unsigned int nameLen;

	unsigned long id;
	void *rootPT;

	spinlock_t lock;
	int exitValue;

	thread_t mainThread;
};

#endif