#include <sched/queue.h>

#include <stdint.h>
#include <stddef.h>
#include <sched/spinlock.h>
#include <sched/thread.h>
#include <sched/signal.h>
#include <sched/process.h>

struct ThreadQueueEntry *threadQueuePop(struct ThreadInfoQueue *queue) {
	struct ThreadQueueEntry *qe = queue->first;
	if (!qe) {
		return NULL;
	}
	queue->first = qe->nextThread;
	if (!queue->first) {
		//queue is now empty
		queue->last = NULL;
	} else {
		queue->first->prevThread = NULL;
	}
	queue->nrofThreads -= 1;
	qe->queue = NULL;
	return qe;
}

void threadQueuePush(struct ThreadInfoQueue *queue, struct ThreadInfo *thread) {
	struct ThreadQueueEntry *qe = thread->queueEntry;
	qe->queue = queue;
	qe->nextThread = NULL;
	if (queue->last) {
		queue->last->nextThread = qe;
		qe->prevThread = queue->last;
	} else {
		queue->first = qe;
		qe->prevThread = NULL;
	}
	queue->last = qe;
	queue->nrofThreads += 1;
}

void threadQueuePushFront(struct ThreadInfoQueue *queue, struct ThreadInfo *thread) {
	struct ThreadQueueEntry *qe = thread->queueEntry;
	qe->queue = queue;
	qe->prevThread = NULL;
	qe->nextThread = queue->first;
	queue->first = qe;
	if (!queue->last) {
		queue->last = qe;
	}
	queue->nrofThreads += 1;
}

void threadQueueRemove(struct ThreadInfo *thread) {
	/*struct ThreadInfoQueue *queue = thread->queueEntry->queue;
	struct ThreadInfo *next = thread->queueEntry->nextThread;
	struct ThreadInfo *prev = thread->queueEntry->prevThread;
	struct SigRegs *regs = NULL;
	while (true) {
		if (queue) {
			if (prev) {
				prev->queueEntry->nextThread = next;
			} else {
				queue->first = next;
			}
			if (next) {
				next->queueEntry->prevThread = prev;
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
	thread->queueEntry->queue = NULL;*/
	struct ThreadQueueEntry *qe = thread->queueEntry;
	while (qe) {
		if (qe->queue) {
			if (qe->prevThread) {
				qe->prevThread->nextThread = qe->nextThread;
			} else {
				qe->queue->first = qe->nextThread;
			}
			if (qe->nextThread) {
				qe->nextThread->prevThread = qe->prevThread;
			} else {
				qe->queue->last = qe->prevThread;
			}
			qe->queue->nrofThreads -= 1;
		}
		qe->queue = NULL;
		qe = qe->prevQueueEntry;
	}
}