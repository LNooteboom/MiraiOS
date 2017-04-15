#ifndef INCLUDE_SCHED_QUEUE_H
#define INCLUDE_SCHED_QUEUE_H

#include <sched/thread.h>
#include <sched/spinlock.h>

struct threadInfoQueue {
	struct threadInfo *first;
	struct threadInfo *last;
	int nrofThreads;
	spinlock_t lock;
};

struct threadInfo *threadQueuePop(struct threadInfoQueue *queue);

void threadQueuePush(struct threadInfoQueue *queue, struct threadInfo *thread);

void threadQueuePushFront(struct threadInfoQueue *queue, struct threadInfo *thread);

#endif