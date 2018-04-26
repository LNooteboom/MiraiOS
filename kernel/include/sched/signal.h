#ifndef INCLUDE_SCHED_SIGNAL_H
#define INCLUDE_SCHED_SIGNAL_H

#include <uapi/types.h>

#include <uapi/signal.h>
#include <arch/signal.h>

#define NROF_SIGNALS	(SIGRTMAX + 1)
#define SIG_STATUS_MASK		0x000FF
#define SIG_EXITED			0x00100
#define SIG_CONTINUED		0x00200
#define SIG_SIGNALED		0x00300
#define SIG_STOPPED			0x00400
#define SIG_TERMSIGNO_MASK	0xFF000
#define SIG_TERMSIGNO_SHIFT	12

struct PendingSignal {
	struct PendingSignal *next;
	struct PendingSignal *prev;

	siginfo_t info;
};

void handleSignal(thread_t curThread, unsigned long *irqStack);

int sendSignal(struct Process *proc, int sigNum);
int sendSignalToPid(pid_t pid, int sigNum);
int sendSignalToGroup(pid_t pgid, int sigNum);

#endif