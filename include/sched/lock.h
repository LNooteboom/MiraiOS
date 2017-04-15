#ifndef INCLUDE_SCHED_LOCK_H
#define INCLUDE_SCHED_LOCK_H

#include <sched/spinlock.h>
#include <sched/queue.h>

struct semaphore {
	spinlock_t lock;
	int value;
	struct threadInfoQueue queue;
};

typedef struct semaphore semaphore_t;

void semInit(semaphore_t *semaphore, int value);

void semWait(semaphore_t *semaphore);

void semSignal(semaphore_t *semaphore);

#endif