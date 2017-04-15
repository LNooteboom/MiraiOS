#ifndef INCLUDE_SCHED_READYQUEUE_H
#define INCLUDE_SCHED_READYQUEUE_H

#include <sched/thread.h>

#define NROF_QUEUE_PRIORITIES	8

thread_t readyQueuePop(void);

void readyQueuePush(thread_t thread);

void readyQueuePushFront(thread_t thread);

thread_t readyQueueExchange(thread_t thread);

#endif