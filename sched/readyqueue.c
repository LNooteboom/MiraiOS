#include <sched/readyqueue.h>

#include <sched/queue.h>
#include <sched/thread.h>
#include <stdint.h>

static struct threadInfoQueue readyList[NROF_QUEUE_PRIORITIES];
spinlock_t readyListLock;

static void calcNewPriority(thread_t thread) {
	int timeslice = TIMESLICE_BASE << thread->priority;
	int remaining = thread->jiffiesRemaining;
	if (thread->fixedPriority || (remaining > 0 && remaining < (timeslice / 2) )) {
		//if thread used up more than half of its timeslice, but wasn't preempted
		//Same priority
		//Do nothing
	} else if (remaining < 1) {
		//If thread was preempted
		//Lower priority (increment the priority value)
		if (thread->priority < NROF_QUEUE_PRIORITIES - 1) {
			thread->priority += 1;
		}
	} else if (remaining >= (timeslice / 2)) {
		//If thread used up less than half of its timeslice
		//Higher priority
		if (thread->priority > 1) {
			thread->priority -= 1;
		}
	}
}

thread_t readyQueuePop(void) {
	thread_t ret;
	acquireSpinlock(&readyListLock);
	for (int i = 0; i < NROF_QUEUE_PRIORITIES; i++) {
		ret = threadQueuePop(&readyList[i]);
		if (ret) {
			ret->jiffiesRemaining = TIMESLICE_BASE << i;
			break;
		}
	}
	releaseSpinlock(&readyListLock);
	return ret;
}

void readyQueuePush(thread_t thread) {
	calcNewPriority(thread);
	acquireSpinlock(&readyListLock);
	threadQueuePush(&readyList[thread->priority], thread);
	releaseSpinlock(&readyListLock);
}

void readyQueuePushFront(thread_t thread) {
	//Use the same priority
	acquireSpinlock(&readyListLock);
	threadQueuePushFront(&readyList[thread->priority], thread);
	releaseSpinlock(&readyListLock);
}

thread_t readyQueueExchange(thread_t thread, bool front) {
	if (front) {
		acquireSpinlock(&readyListLock);
		threadQueuePushFront(&readyList[thread->priority], thread);
	} else {
		calcNewPriority(thread);
		acquireSpinlock(&readyListLock);
		threadQueuePush(&readyList[thread->priority], thread);
	}

	thread_t ret;
	for (int i = 0; i < NROF_QUEUE_PRIORITIES; i++) {
		ret = threadQueuePop(&readyList[i]);
		if (ret) {
			ret->jiffiesRemaining = TIMESLICE_BASE << i;
			break;
		}
	}
	if (!ret) {
		ret = thread;
	}
	releaseSpinlock(&readyListLock);
	return ret;
}

