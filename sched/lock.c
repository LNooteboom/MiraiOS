#include <sched/lock.h>

#include <sched/spinlock.h>
#include <sched/readyqueue.h>


void semInit(semaphore_t *semaphore, int value) {
	semaphore->value = value;
}

void semWait(semaphore_t *semaphore) {
	thread_t curThread = getCurrentThread();
	acquireSpinlock(&curThread->lock);
	acquireSpinlock(&semaphore->lock);
	semaphore->value -= 1;
	if (semaphore->value >= 0) {
		releaseSpinlock(&semaphore->lock);
		releaseSpinlock(&curThread->lock);
		return;
	}
	threadQueuePush(&semaphore->queue, curThread);
	releaseSpinlock(&semaphore->lock);
	kthreadStop();
}

void semSignal(semaphore_t *semaphore) {
	acquireSpinlock(&semaphore->lock);
	thread_t freedThread = threadQueuePop(&semaphore->queue);
	semaphore->value += 1;
	releaseSpinlock(&semaphore->lock);
	if (freedThread) {
		acquireSpinlock(&freedThread->lock);
		freedThread->state = THREADSTATE_SCHEDWAIT;
		readyQueuePush(freedThread);
		releaseSpinlock(&freedThread->lock);
	}
}