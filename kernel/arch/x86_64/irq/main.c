#include <irq.h>

#include <stdint.h>
#include <mm/heap.h>
#include <sched/spinlock.h>
#include <arch/idt.h>
#include "exception.h"

extern void undefinedInterrupt(void);
extern void dummyInterrupt(void);

extern void initIrqStubs(void);

void archInitInterrupts(void) {
	initIDT();

	initExceptions();
	initIrqStubs();

	for (int i = 0xE0; i < 0x100; i++) {
		if (i >= 0xF0) {
			mapIdtEntry(undefinedInterrupt, i, 0);
		} else {
			mapIdtEntry(dummyInterrupt, i, 0);
		}
	}
}