#ifndef INCLUDE_ARCH_SIGNAL_H
#define INCLUDE_ARCH_SIGNAL_H

#include <stdbool.h>
#include <sched/thread.h>
#include <sched/queue.h>

struct SigRegs {
	//regs must be in reverse order
	uint64_t r15;
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t rbp;
	uint64_t rbx;
	uint64_t r11;
	uint64_t r10;
	uint64_t r9;
	uint64_t r8;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rdx;
	uint64_t rcx;
	uint64_t rax;

	uint64_t rip;
	uint64_t rsp;
	uint64_t rflags;

	struct SigRegs *next;

	enum threadState tState;
	bool floatsUsed;

	struct ThreadInfoQueue *queue;
	struct ThreadInfo *nextThread;
	struct ThreadInfo *prevThread;
	sigset_t sigMask;

	char fxsaveArea[512] __attribute__((aligned(16)));
};

#endif