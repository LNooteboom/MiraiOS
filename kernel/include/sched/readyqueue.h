#ifndef INCLUDE_SCHED_READYQUEUE_H
#define INCLUDE_SCHED_READYQUEUE_H

#include <sched/thread.h>

#define NROF_QUEUE_PRIORITIES	8

/*
Pops a thread from the current cpu's readyqueue
*/
thread_t readyQueuePop(void);

/*
Pushes a thread to the current cpu's readyqueue
*/
void readyQueuePush(struct ThreadQueueEntry *qe);

/*
Attempts to find a thread with a higher priority value than the current thread.
*/
thread_t readyQueueExchange(thread_t thread, bool front);

#endif