#ifndef INCLUDE_SCHED_QUEUE_H
#define INCLUDE_SCHED_QUEUE_H

#include <sched/thread.h>
#include <sched/spinlock.h>

struct ThreadInfoQueue {
	struct ThreadInfo *first;
	struct ThreadInfo *last;
	int nrofThreads;
};

/*
Pops a thread from the front of a queue.
*/
struct ThreadInfo *threadQueuePop(struct ThreadInfoQueue *queue);

/*
Pushes a thread to the back of a queue
*/
void threadQueuePush(struct ThreadInfoQueue *queue, struct ThreadInfo *thread);

/*
Pushes a thread to the front of a queue
*/
void threadQueuePushFront(struct ThreadInfoQueue *queue, struct ThreadInfo *thread);

#endif