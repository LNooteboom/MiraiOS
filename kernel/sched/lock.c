#include <sched/lock.h>

#include <sched/spinlock.h>
#include <sched/readyqueue.h>
#include <stddef.h>
#include <mm/memset.h>

void semInit(semaphore_t *semaphore, int value) {
	memset(semaphore, 0, sizeof(semaphore_t));
	semaphore->value = value;
}

void semWait(semaphore_t *semaphore) {
	thread_t curThread = getCurrentThread();
	acquireSpinlock(&curThread->lock);
	acquireSpinlock(&semaphore->lock);
	
	if (semaphore->value > 0) {
		semaphore->value--;
		releaseSpinlock(&semaphore->lock);
		releaseSpinlock(&curThread->lock);
		return;
	}

	threadQueuePush(&semaphore->queue, curThread);
	releaseSpinlock(&semaphore->lock);
	curThread->state = THREADSTATE_LOCKWAIT;
	kthreadStop();
}

void semSignal(semaphore_t *semaphore) {
	acquireSpinlock(&semaphore->lock);
	if (semaphore->value > 0) {
		semaphore->value++;
		goto end;
	}
	struct ThreadQueueEntry *freed = threadQueuePop(&semaphore->queue);
	if (freed) {
		readyQueuePush(freed);
	} else {
		semaphore->value++;
	}

	end:
	releaseSpinlock(&semaphore->lock);
}