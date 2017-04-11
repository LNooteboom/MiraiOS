#include <sched/thread.h>

#include <stdint.h>
#include <mm/paging.h>
#include <apic.h>
#include <print.h>
#include "readyqueue.h"

#define THREAD_STACK_SIZE	0x2000

extern void migrateMainStack(thread_t mainThread);

extern void kthreadInit(thread_t thread, void *(*start)(void *), void *arg);

static void deallocThread(thread_t thread) {
	uintptr_t stackBottom = (uintptr_t)(thread) - (THREAD_STACK_SIZE - sizeof(struct threadInfo));
	deallocPages((void*)stackBottom, THREAD_STACK_SIZE);
}

int kthreadCreate(thread_t *thread, void *(*start)(void *), void *arg) {
	//alloc thread stack
	uintptr_t stackBottom = (uintptr_t)(allocKPages(THREAD_STACK_SIZE, PAGE_FLAG_WRITE | PAGE_FLAG_INUSE));
	if (!stackBottom) {
		return THRD_NOMEM;
	}
	struct threadInfo *thrd = (thread_t)(stackBottom + THREAD_STACK_SIZE - sizeof(struct threadInfo));
	*thread = thrd;

	thrd->state = THREADSTATE_SCHEDWAIT;
	thrd->priority = 0;
	thrd->jiffiesRemaining = 1;
	thrd->detached = false;

	kthreadInit(thrd, start, arg);

	//now push it to the scheduling queue
	readyQueuePush(thrd);
	
	return THRD_SUCCESS;
}

int kthreadCreateFromMain(thread_t *thread) {
	//alloc thread stack
	uintptr_t stackBottom = (uintptr_t)(allocKPages(THREAD_STACK_SIZE, PAGE_FLAG_WRITE | PAGE_FLAG_INUSE));
	if (!stackBottom) {
		return THRD_NOMEM;
	}
	struct threadInfo *thrd = (thread_t)(stackBottom + THREAD_STACK_SIZE - sizeof(struct threadInfo));
	*thread = thrd;
	migrateMainStack(thrd);

	thrd->state = THREADSTATE_RUNNING;
	thrd->priority = 0;
	thrd->jiffiesRemaining = 1;
	thrd->detached = true;

	//uint32_t cpu = getCPU();
	//cpuInfos[cpu].currentThread = thrd;
	setCurrentThread(thrd);
	
	return THRD_SUCCESS;
}

thread_t kthreadSwitch(void) {
	thread_t oldThread = getCurrentThread();
	thread_t newThread = readyQueuePop();
	if (!newThread || newThread == oldThread) {
		newThread = oldThread;
	} else {
		oldThread->state = THREADSTATE_SCHEDWAIT;
		readyQueuePush(oldThread);
	}
	newThread->state = THREADSTATE_RUNNING;
	setCurrentThread(newThread);
	return newThread;
}

void kthreadJoin(thread_t thread, void **returnValue) {
	acquireSpinlock(&(thread->lock));
	if (thread->state != THREADSTATE_FINISHED) {
		thread_t curThread = getCurrentThread();
		curThread->nextThread = NULL;
		if (thread->joinLast) {
			curThread->prevThread = thread->joinLast;
			thread->joinLast->nextThread = curThread;
			thread->joinLast = curThread;
		} else {
			curThread->prevThread = NULL;
			thread->joinFirst = curThread;
			thread->joinLast = curThread;
		}
		releaseSpinlock(&(thread->lock));
		kthreadStop();
		acquireSpinlock(&(thread->lock));
	}

	*returnValue = thread->returnValue;
	releaseSpinlock(&(thread->lock));
	deallocThread(thread);
}

void kthreadFreeJoined(thread_t thread) {
	//spinlock is already acquired
	thread_t curFreeThread = thread->joinFirst;
	while (curFreeThread) {
		acquireSpinlock(&(curFreeThread->lock));
		thread_t nextFreeThread = curFreeThread->nextThread;
		curFreeThread->state = THREADSTATE_SCHEDWAIT;
		readyQueuePush(curFreeThread);
		releaseSpinlock(&(curFreeThread->lock));
		curFreeThread = nextFreeThread;
	}
	thread->joinFirst = NULL;
	thread->joinLast = NULL;
}