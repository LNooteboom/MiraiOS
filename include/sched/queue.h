#ifndef INCLUDE_SCHED_QUEUE_H
#define INCLUDE_SCHED_QUEUE_H

#include <sched/thread.h>
#include <sched/spinlock.h>

struct threadInfoQueue {
	struct threadInfo *first;
	struct threadInfo *last;
	int nrofThreads;
};

/*
Pops a thread from the front of a queue.
*/
struct threadInfo *threadQueuePop(struct threadInfoQueue *queue);

/*
Pushes a thread to the back of a queue
*/
void threadQueuePush(struct threadInfoQueue *queue, struct threadInfo *thread);

/*
Pushes a thread to the front of a queue
*/
void threadQueuePushFront(struct threadInfoQueue *queue, struct threadInfo *thread);

#endif