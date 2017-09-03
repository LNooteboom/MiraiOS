#ifndef INCLUDE_SCHED_LOCK_H
#define INCLUDE_SCHED_LOCK_H

#include <sched/spinlock.h>
#include <sched/queue.h>

struct Semaphore {
	spinlock_t lock;
	int value;
	struct ThreadInfoQueue queue;
};

typedef struct Semaphore semaphore_t;

/*
Initializes a semaphore
*/
void semInit(semaphore_t *semaphore, int value);

/*
Decrements the semaphore and blocks if the result is negative
*/
void semWait(semaphore_t *semaphore);

/*
Increments the semaphore and frees a thread if the value was negative
*/
void semSignal(semaphore_t *semaphore);

#endif