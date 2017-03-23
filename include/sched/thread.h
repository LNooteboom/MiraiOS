#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <stdbool.h>

#define THRD_SUCCESS	0
#define THRD_NOMEM		-1
#define THRD_BAD_PARAM	-2

enum threadState {
	THREADSTATE_RUNNING,
	THREADSTATE_SCHEDWAIT,
	THREADSTATE_MUTEXWAIT,
	THREADSTATE_SLEEP,
	THREADSTATE_FINISHED
};

struct threadInfo {
	uintptr_t stackPointer;
	enum threadState state;
	struct threadInfo *nextThread;
	int priority;
	int jiffiesRemaining;
	int sleepTime;
	bool detached;
};

typedef struct threadInfo * thread_t;

int createKernelThread(thread_t *thread, void *(*start)(void *), void *arg);

void kthreadInit(thread_t thread, void *(*start)(void *), void *arg);

#endif