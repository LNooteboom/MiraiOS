#include <irq.h>

#include <stdint.h>
#include <mm/heap.h>
#include <sched/spinlock.h>
#include "idt.h"
#include "exception.h"

#define NROF_BITMAP_IRQS	128

spinlock_t irqBitmapLock;
uint32_t irqBitmap[NROF_BITMAP_IRQS / 4];

void initInterrupts(void) {
	initIDT();
	initExceptions();
	irqBitmap[0] = ~0; //set exceptions as used
}

interrupt_t allocIrqVec(void) {
	int bitPos;
	int arrayPos;
	bool foundVec = false;
	acquireSpinlock(&irqBitmapLock);
	for (unsigned int i = 0; i < NROF_BITMAP_IRQS / 4; i++) {
		if (irqBitmap[i] != (uint32_t)(~0)) {
			bitPos = 0;
			uint32_t bit = irqBitmap[i];
			while (bit & 1) {
				bitPos++;
				bit >>= 1;
			}
			irqBitmap[i] |= (1 << bitPos);
			arrayPos = i;
			foundVec = true;
			break;
		}
	}
	releaseSpinlock(&irqBitmapLock);
	if (!foundVec) {
		return 0;
	}
	return (arrayPos * 32) + bitPos;
}
void deallocIrqVec(interrupt_t vec) {
	int bitPos = vec % 32;
	int arrayPos = vec / 32;
	acquireSpinlock(&irqBitmapLock);
	irqBitmap[arrayPos] &= ~(1 << bitPos);
	releaseSpinlock(&irqBitmapLock);
}