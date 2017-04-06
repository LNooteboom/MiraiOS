#ifndef INCLUDE_SCHED_THREAD_H
#define INCLUDE_SCHED_THREAD_H

#include <stdint.h>
#include <stdbool.h>
#include <spinlock.h>

#define THRD_SUCCESS	0
#define THRD_NOMEM		-1
#define THRD_BAD_PARAM	-2

enum threadState {
	THREADSTATE_FINISHED,
	THREADSTATE_RUNNING,
	THREADSTATE_SCHEDWAIT,
	THREADSTATE_MUTEXWAIT,
	THREADSTATE_SLEEP
};

struct threadInfo {
	uintptr_t stackPointer;
	void *returnValue;
	enum threadState state;
	spinlock_t lock;

	struct threadInfo *nextThread;
	struct threadInfo *prevThread;
	int priority;
	int jiffiesRemaining;
	int sleepTime;
	bool detached;

	struct threadInfo *joinFirst;
	struct threadInfo *joinLast;
};

typedef struct threadInfo *thread_t;

int createKernelThread(thread_t *thread, void *(*start)(void *), void *arg);

void joinKernelThread(thread_t thread, void **returnValue);

thread_t getCurrentThread(void);
void setCurrentThread(thread_t thread);

int createThreadFromMain(thread_t *thread);

void kthreadStop(void);

#endif