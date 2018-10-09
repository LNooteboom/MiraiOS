
#include <sched/queue.h>
#include <sched/readyqueue.h>
#include <sched/thread.h>
#include <stdbool.h>
#include <stddef.h>

struct ThreadInfoQueue sleepQueue;
spinlock_t sleepQueueLock;

void kthreadSleep(unsigned long millis) {
	unsigned long sleepTime = millis * 1000 / JIFFY_HZ;
	if (sleepTime == 0) {
		return;
	}
	thread_t curThread = getCurrentThread();
	acquireSpinlock(&curThread->lock);
	curThread->queueEntry->waitData = (void *)sleepTime;
	curThread->state = THREADSTATE_SLEEP;
	threadQueuePush(&sleepQueue, curThread);
	kthreadStop();
}

bool sleepSkipTime(thread_t curThread) {
	/*thread_t thrd = sleepQueue.first;
	bool higherThreadReleased = false;
	while (thrd) {
		if (thrd != curThread) {
			acquireSpinlock(&thrd->lock);
		}
		thread_t next = thrd->queueEntry->nextThread;
		thrd->sleepTime--;
		if (thrd->sleepTime == 0) {
			//remove it from the sleep queue
			if (thrd->queueEntry->prevThread) {
				thrd->queueEntry->prevThread->queueEntry->nextThread = thrd->queueEntry->nextThread;
			} else {
				sleepQueue.first = thrd->queueEntry->nextThread;
			}
			if (thrd->queueEntry->nextThread) {
				thrd->queueEntry->nextThread->queueEntry->prevThread = thrd->queueEntry->prevThread;
			} else {
				sleepQueue.last = thrd->queueEntry->prevThread;
			}

			//add it to the readyqueue
			readyQueuePush(thrd);

			if (!curThread || thrd->priority < curThread->priority) {
				higherThreadReleased = true;
			}
		}
		if (thrd != curThread) {
			releaseSpinlock(&thrd->lock);
		}
		thrd = next;
	}
	return higherThreadReleased;*/
	bool higherThreadReleased = false;
	struct ThreadQueueEntry *qe = sleepQueue.first;
	while (qe) {
		thread_t thrd = qe->thread;
		if (thrd != curThread) {
			acquireSpinlock(&thrd->lock);
		}
		qe->waitData = (void *)((unsigned long)(qe->waitData) - 1);
		if ((unsigned long)(qe->waitData) == 0) {
			if (qe->prevThread) {
				qe->prevThread->nextThread = qe->nextThread;
			} else {
				sleepQueue.first = qe->nextThread;
			}
			if (qe->nextThread) {
				qe->nextThread->prevThread = qe->prevThread;
			} else {
				sleepQueue.last = qe->prevThread;
			}
			qe->queue->nrofThreads -= 1;
			qe->queue = NULL;

			//add it to the readyqueue
			readyQueuePush(qe);

			if (!curThread || thrd->priority < curThread->priority) {
				higherThreadReleased = true;
			}
		}

		if (thrd != curThread) {
			releaseSpinlock(&thrd->lock);
		}
		qe = qe->nextThread;
	}
	return higherThreadReleased;
}

int sysSleep(uint64_t seconds, uint32_t nanoSeconds) {
	uint64_t millis = seconds * 1000 + nanoSeconds / 1000000;
	kthreadSleep(millis);
	return 0;
}