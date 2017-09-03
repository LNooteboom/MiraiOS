
#include <sched/queue.h>
#include <sched/readyqueue.h>
#include <sched/thread.h>
#include <stdbool.h>
#include <stddef.h>

struct threadInfoQueue sleepQueue;
spinlock_t sleepQueueLock;

void kthreadSleep(unsigned long millis) {
	if (millis == 0) {
		return;
	}
	thread_t curThread = getCurrentThread();
	acquireSpinlock(&curThread->lock);
	curThread->sleepTime = millis * 1000 / JIFFY_HZ;
	curThread->state = THREADSTATE_SLEEP;
	threadQueuePush(&sleepQueue, curThread);
	kthreadStop();
}

bool sleepSkipTime(thread_t curThread) {
	thread_t thrd = sleepQueue.first;
	bool higherThreadReleased = false;
	while (thrd) {
		if (thrd != curThread) {
			acquireSpinlock(&thrd->lock);
		}
		thread_t next = thrd->nextThread;
		thrd->sleepTime--;
		if (thrd->sleepTime == 0) {
			//remove it from the sleep queue
			if (thrd->prevThread) {
				thrd->prevThread->nextThread = thrd->nextThread;
			} else {
				sleepQueue.first = thrd->nextThread;
			}
			if (thrd->nextThread) {
				thrd->nextThread->prevThread = thrd->prevThread;
			} else {
				sleepQueue.last = thrd->prevThread;
			}

			//add it to the readyqueue
			thrd->state = THREADSTATE_SCHEDWAIT;
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
	return higherThreadReleased;
}