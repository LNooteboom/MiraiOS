#ifndef INCLUDE_SCHED_SIGNAL_H
#define INCLUDE_SCHED_SIGNAL_H

#include <uapi/types.h>

#include <uapi/signal.h>
#include <arch/signal.h>

#define NROF_SIGNALS	(SIGRTMAX + 1)

struct PendingSignal {
	struct PendingSignal *next;
	struct PendingSignal *prev;

	siginfo_t info;
};

void handleSignal(thread_t curThread, unsigned long *irqStack);

int sendSignal(struct Process *proc, int sigNum);
int sendSignalToPid(pid_t pid, int sigNum);

#endif