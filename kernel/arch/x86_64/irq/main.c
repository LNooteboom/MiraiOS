#include <irq.h>

#include <stdint.h>
#include <mm/heap.h>
#include <sched/spinlock.h>
#include <arch/idt.h>
#include "exception.h"
#include <sched/thread.h>
#include <sched/process.h>

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

void excExit(int excType, void *addr) {
	struct Process *proc = getCurrentThread()->process;
	proc->exitInfo.si_signo = SIGILL;
	proc->exitInfo.si_code = excType;
	proc->exitInfo.si_addr = addr;
	signalExit();
}

void pfExit(void *cr2, void *addr) {
	(void)cr2;
	struct Process *proc = getCurrentThread()->process;
	proc->exitInfo.si_signo = SIGSEGV;
	proc->exitInfo.si_addr = addr;
	signalExit();
}

void sigretExit(void) {
	struct Process *proc = getCurrentThread()->process;
	proc->exitInfo.si_signo = SIGABRT;
	signalExit();
}