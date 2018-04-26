#include <sched/signal.h>
#include <sched/spinlock.h>
#include <sched/process.h>
#include <sched/pgrp.h>
#include <arch/cpu.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <errno.h>
#include <userspace.h>
#include <print.h>

int sigprocmask(int how, const sigset_t *newset, sigset_t *oldset, struct Process *proc);

#define COPY(regName)	regs->regName = irqStack[i++]
static void copyRegs(struct SigRegs *regs, unsigned long *irqStack) {
	int i = 0;
	COPY(r15);
	COPY(r14);
	COPY(r13);
	COPY(r12);
	COPY(rbp);
	COPY(rbx);

	COPY(r11);
	COPY(r10);
	COPY(r9);
	COPY(r8);
	COPY(rdi);
	COPY(rsi);
	COPY(rdx);
	COPY(rcx);
	COPY(rax);

	COPY(rip);
	i++; //cs
	COPY(rflags);
	COPY(rsp);
}

static struct PendingSignal *getPendingSignal(struct Process *proc) {
	struct PendingSignal *sig = proc->sigFirst;
	while (sig) {
		if ( !(proc->sigMask & (1 << sig->info.si_signo))) {
			//remove sig from list
			if (sig->prev) {
				sig->prev->next = sig->next;
			} else {
				proc->sigFirst = sig->next;
			}
			if (sig->next) {
				sig->next->prev = sig->prev;
			} else {
				proc->sigLast = sig->prev;
			}
			return sig;
		}
		sig = sig->next;
	}
	return sig;
}

void handleSignal(thread_t curThread, unsigned long *irqStack) {
	struct Process *proc = curThread->process;
	if (!proc || !proc->sigResched) {
		return;
	}
	acquireSpinlock(&proc->lock);
	proc->sigResched = false;

	struct PendingSignal *sig = getPendingSignal(proc);
	if (!sig) {
		printk("[SCHED] sigResched set but no available signal!\n");
		return;
	}

	int sigNum = sig->info.si_signo;
	proc->sigPending &= ~(1 << sigNum);
	proc->sigNrPending--;
	if (proc->sigAct[sigNum].sa_handler == SIG_IGN) {
		goto release; //ignore signal
	}

	proc->sigMask = proc->sigAct[sigNum].sa_mask;

	struct SigRegs *regs = proc->sigRegStack;
	copyRegs(regs, irqStack);
	regs->sigMask = proc->sigMask;
	regs->floatsUsed = curThread->floatsUsed;
	if (curThread->floatsUsed) {
		memcpy(regs->fxsaveArea, curThread->fxsaveArea, 512);
	}

	unsigned long *iret = irqStack + 15;
	unsigned long *stack;
	if (proc->sigAltStack && !proc->sigRegStack) {
		stack = (unsigned long *)((char *)proc->sigAltStack + proc->sigAltStackSize);
	} else {
		if (iret[0] < 0xffffffff80000000) {
			stack = (unsigned long *)iret[3];
		} else {
			stack = *((unsigned long **)curThread - 1);
		}

		stack -= 128 / 8;//account for red zone
	}

	//setup stack
	size_t sz = align(sizeof(siginfo_t), 8);
	stack -= sz / 8;
	memcpy(stack, &sig->info, sizeof(siginfo_t));
	
	
	if (proc->sigAct[sigNum].sa_handler) {
		curThread->sigDepth++;
		tssSetRSP0((void *)(iret + 5));
		curThread->stackPointer = (uintptr_t)(iret + 5);
		//return to signal handler
		iret[4] = 0x23;
		iret[3] = (long)stack; //rsp
		iret[2] |= (1 << 9); //rflags
		iret[1] = 0x2B;
		iret[0] = (long)proc->sigAct[sigNum].saTrampoline; //rip
		iret[-1] = (long)proc->sigAct[sigNum].sa_handler; //rax = handler
		iret[-3] = 0; //rdx = context
		iret[-4] = sigNum; //rdi = sigNum
		iret[-5] = (long)stack; //rsi = siginfo
	} else {
		//perform default action
		//TODO add thread stop and core dump
		//iret[-4] = -1;
		//iret[0] = (long)sysExit; //return to sysExit(-1)
		memcpy(&proc->exitInfo, &sig->info, sizeof(siginfo_t));
		proc->exitInfo.si_status = SIG_SIGNALED | (sigNum << SIG_TERMSIGNO_SHIFT);
		iret[0] = (long)signalExit;
		iret[1] = 8; //kernel cs
		iret[3] = (long)curThread; //rsp
		iret[4] = 0x10; //kernel ss
	}

	releaseSpinlock(&proc->lock);
	sigprocmask(SIG_SETMASK, &proc->sigAct[sigNum].sa_mask, NULL, proc);
	goto out;

	release:
	releaseSpinlock(&proc->lock);
	out:
	kfree(sig);
}

void sigRet(struct SigRegs **regs) {
	thread_t curThread = getCurrentThread();
	struct Process *proc = curThread->process;
	acquireSpinlock(&proc->lock);
	*regs = proc->sigRegStack;
	proc->sigRegStack = proc->sigRegStack->next;
	
	releaseSpinlock(&proc->lock);

	sigprocmask(SIG_SETMASK, &(*regs)->sigMask, NULL, proc);
	acquireSpinlock(&curThread->lock);

	curThread->nextThread = (*regs)->nextThread;
	curThread->prevThread = (*regs)->prevThread;
	curThread->queue = (*regs)->queue;
}

static void notifyProcess(struct Process *proc, int sigNum, struct SigRegs *regs) {
	while (proc->sigResched) {
		//wait until previous signal is received
		releaseSpinlock(&proc->lock);
		acquireSpinlock(&proc->lock);
	}

	thread_t mainThread = proc->mainThread;
	acquireSpinlock(&mainThread->lock);
	regs->queue = mainThread->queue;
	regs->nextThread = mainThread->nextThread;
	regs->prevThread = mainThread->prevThread;
	regs->tState = mainThread->state;

	regs->next = proc->sigRegStack;
	proc->sigRegStack = regs;
	proc->sigResched = true;
	if (mainThread->state != THREADSTATE_SCHEDWAIT) {
		if (mainThread->state != THREADSTATE_RUNNING) {
			if (!(proc->sigAct[sigNum].sa_handler)) {
				//threadQueueRemove(mainThread);
			}
			readyQueuePush(mainThread);
		}
		lapicSendIPI(cpuInfos[mainThread->cpuAffinity].apicID, RESCHED_VEC, IPI_FIXED);
	}
	releaseSpinlock(&mainThread->lock);
}

int sendSignal(struct Process *proc, int sigNum) {
	if (proc->state == PROCSTATE_FINISHED) {
		return -ESRCH;
	}
	if (proc->pid == 1 && !(proc->sigAct[sigNum].saTrampoline)) {
		return -EPERM; //do not send unhandled signals to init
	}
	
	struct PendingSignal *ps = kzalloc(sizeof(*ps));
	if (!ps) {
		return -ENOMEM;
	}

	ps->info.si_signo = sigNum;

	acquireSpinlock(&proc->lock);
	if (proc->sigLast) {
		proc->sigLast->next = ps;
	} else {
		proc->sigFirst = ps;
	}
	ps->prev = proc->sigLast;
	proc->sigLast = ps;

	proc->sigPending |= (1 << sigNum);
	proc->sigNrPending++;

	if ( !(proc->sigMask & (1 << sigNum))) {
		struct SigRegs *regs = kmalloc(sizeof(*regs));
		notifyProcess(proc, sigNum, regs);
	}

	releaseSpinlock(&proc->lock);
	return 0;
}

int sysSigHandler(int sigNum, struct sigaction *action, struct sigaction *oldAction) {
	int error = 0;
	if (sigNum < 0 || sigNum > SIGRTMAX || sigNum == SIGKILL || sigNum == SIGSTOP) {
		return -EINVAL;
	}
	struct Process *proc = getCurrentThread()->process;
	acquireSpinlock(&proc->lock);

	if (oldAction) {
		error = validateUserPointer(oldAction, sizeof(*oldAction));
		if (error) goto release;
		memcpy(oldAction, &proc->sigAct[sigNum], sizeof(*oldAction));
	}
	if (!action) goto release;

	error = validateUserPointer(action, sizeof(*action));
	if (error) goto release;

	memcpy(&proc->sigAct[sigNum], action, sizeof(*action));

	release:
	releaseSpinlock(&proc->lock);
	return error;
}

int sendSignalToPid(pid_t pid, int sigNum) {
	struct Process *proc = getProcFromPid(pid);
	if (!proc) {
		return -ESRCH;
	}
	return sendSignal(proc, sigNum);
}

int sendSignalToGroup(pid_t pgid, int sigNum) {
	struct PGroup *grp = getPGroup(pgid);
	if (!grp) {
		return -ESRCH;
	}
	struct Process *proc = grp->first;
	int error = 0;
	while (proc && !error) {
		struct Process *next = proc->grpNext;
		error = sendSignal(proc, sigNum);
		proc = next;
	}
	return error;
}

int sigprocmask(int how, const sigset_t *newset, sigset_t *oldset, struct Process *proc) {
	int error = 0;
	acquireSpinlock(&proc->lock);

	sigset_t newMask = proc->sigMask;
	if (oldset) {
		*oldset = newMask;
	}
	if (!newset) {
		goto release;
	}
	switch (how) {
		case SIG_BLOCK:
			newMask |= *newset;
			break;
		case SIG_UNBLOCK:
			newMask &= ~(*newset);
			break;
		case SIG_SETMASK:
			newMask = *newset;
			break;
		default:
			error = -EINVAL;
			goto release;
	}
	sigset_t unblocked = (newMask | proc->sigMask) ^ proc->sigMask; //get signals that were masked before but not now
	unblocked &= proc->sigPending;
	proc->sigMask = newMask;
	releaseSpinlock(&proc->lock);

	if (!unblocked) {
		goto out;
	}

	int base = 0;
	while (true) {
		int set = __builtin_ffsll(unblocked); //find first set bit (unblocked signal)
		if (!set) {
			break;
		}
		int sigNum = set - 1 + base;
		sendSignal(proc, sigNum);

		base = set;
		unblocked >>= set;
	}

	release:
	releaseSpinlock(&proc->lock);
	out:
	return error;
}

int sysSigprocmask(int how, const sigset_t *newset, sigset_t *oldset) {
	struct Process *proc = getCurrentThread()->process;
	return sigprocmask(how, newset, oldset, proc);
}