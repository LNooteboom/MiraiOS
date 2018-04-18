#include <sched/queue.h>

#include <stdint.h>
#include <stddef.h>
#include <sched/spinlock.h>
#include <sched/thread.h>
#include <sched/signal.h>
#include <sched/process.h>

struct ThreadInfo *threadQueuePop(struct ThreadInfoQueue *queue) {
	struct ThreadInfo *ret = queue->first;
	if (!ret) {
		return NULL;
	}
	queue->first = ret->nextThread;
	if (!queue->first) {
		//queue is now empty
		queue->last = NULL;
	} else {
		queue->first->prevThread = NULL;
	}
	queue->nrofThreads -= 1;
	ret->queue = NULL;
	
	return ret;
}

void threadQueuePush(struct ThreadInfoQueue *queue, struct ThreadInfo *thread) {
	thread->queue = queue;
	thread->nextThread = NULL;
	if (queue->last) {
		queue->last->nextThread = thread;
		thread->prevThread = queue->last;
		queue->last = thread;
	} else {
		queue->first = thread;
		queue->last = thread;
		thread->prevThread = NULL;
	}
	queue->nrofThreads += 1;
}

void threadQueuePushFront(struct ThreadInfoQueue *queue, struct ThreadInfo *thread) {
	thread->queue = queue;
	thread->prevThread = NULL;
	thread->nextThread = queue->first;
	queue->first = thread;
	if (!queue->last) {
		queue->last = thread;
	}
	queue->nrofThreads += 1;
}

void threadQueueRemove(struct ThreadInfo *thread) {
	struct ThreadInfoQueue *queue = thread->queue;
	struct ThreadInfo *next = thread->nextThread;
	struct ThreadInfo *prev = thread->prevThread;
	struct SigRegs *regs = NULL;
	while (true) {
		if (queue) {
			if (prev) {
				prev->nextThread = next;
			} else {
				queue->first = next;
			}
			if (next) {
				next->prevThread = prev;
			} else {
				queue->last = prev;
			}
			queue->nrofThreads--;
		}
		if (!thread->sigDepth) {
			break;
		}

		if (regs) {
			regs = regs->next;
		} else {
			regs = thread->process->sigRegStack;
		}
		if (regs) {
			queue = regs->queue;
			next = regs->nextThread;
			prev = regs->prevThread;
		}
		
		thread->sigDepth--;
	}

	thread->sigDepth = 0;
	thread->queue = NULL;
}