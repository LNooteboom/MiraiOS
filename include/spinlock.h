#ifndef INCLUDE_SPINLOCK_H
#define INCLUDE_SPINLOCK_H

typedef uint8_t spinlock_t;

void acquireSpinlock(spinlock_t *lock);

void releaseSpinlock(spinlock_t *lock);

#endif
