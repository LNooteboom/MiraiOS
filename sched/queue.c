#include <sched/queue.h>

#include <stdint.h>
#include <stddef.h>
#include <sched/spinlock.h>
#include <sched/thread.h>

struct threadInfo *threadQueuePop(struct threadInfoQueue *queue) {
	struct threadInfo *ret = queue->first;
	if (!ret) {
		return NULL;
	}
	queue->first = ret->nextThread;
	if (!queue->first) {
		//queue is now empty
		queue->last = NULL;
	} else {
		queue->first->prevThread = NULL;
	}
	queue->nrofThreads -= 1;
	return ret;
}

void threadQueuePush(struct threadInfoQueue *queue, struct threadInfo *thread) {
	if (!thread) {
		return;
	}
	thread->nextThread = NULL;
	if (queue->last) {
		queue->last->nextThread = thread;
		thread->prevThread = queue->last;
		queue->last = thread;
	} else {
		queue->first = thread;
		queue->last = thread;
		thread->prevThread = NULL;
	}
	queue->nrofThreads += 1;
}

void threadQueuePushFront(struct threadInfoQueue *queue, struct threadInfo *thread) {
	if (!thread) {
		return;
	}
	thread->prevThread = NULL;
	thread->nextThread = queue->first;
	queue->first = thread;
	if (!queue->last) {
		queue->last = thread;
	}
	queue->nrofThreads += 1;
}