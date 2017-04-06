#include "readyqueue.h"

#include "queue.h"
#include <sched/thread.h>
#include <stdint.h>

static struct threadInfoQueue readyList[NROF_QUEUE_PRIORITES];

thread_t readyQueuePop(void) {
	//TODO
	thread_t ret = threadQueuePop(&(readyList[0]));
	//return threadQueuePop(&(readyList[0]));
	if (!ret) {
		//sprint("n");
	}
	return ret;
}

void readyQueuePush(thread_t thread) {
	//TODO
	threadQueuePush(&(readyList[0]), thread);
}

void readyQueuePushFront(thread_t thread) {
	//TODO
	threadQueuePushFront(&(readyList[0]), thread);
}

