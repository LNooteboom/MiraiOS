#include <sched/queue.h>

#include <stdint.h>
#include <stddef.h>
#include <sched/spinlock.h>
#include <sched/thread.h>

struct ThreadInfo *threadQueuePop(struct ThreadInfoQueue *queue) {
	struct ThreadInfo *ret = queue->first;
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

void threadQueuePush(struct ThreadInfoQueue *queue, struct ThreadInfo *thread) {
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

void threadQueuePushFront(struct ThreadInfoQueue *queue, struct ThreadInfo *thread) {
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