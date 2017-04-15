#ifndef INCLUDE_SCHED_SPINLOCK_H
#define INCLUDE_SCHED_SPINLOCK_H

#include <stdint.h>

typedef uint32_t spinlock_t;

void acquireSpinlock(spinlock_t *lock);

void releaseSpinlock(spinlock_t *lock);

#endif
