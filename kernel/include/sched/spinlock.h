#ifndef INCLUDE_SCHED_SPINLOCK_H
#define INCLUDE_SCHED_SPINLOCK_H

#include <stdint.h>

typedef uint32_t spinlock_t;

/*
Acquires a spinlock and disables interrupts
*/
void acquireSpinlock(spinlock_t *lock);

/*
Releases a spinlock and restores the previous interrupt flag setting
*/
void releaseSpinlock(spinlock_t *lock);

#endif
