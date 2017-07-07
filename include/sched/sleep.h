#ifndef INCLUDE_SCHED_SLEEP_H
#define INCLUDE_SCHED_SLEEP_H

#include <sched/thread.h>

/*
Used in inner polling loops to save power
*/
static inline void relax(void) {
	asm ("pause");
}

/*
Sleeps the current thread for a specified number of millis
*/
void kthreadSleep(unsigned long millis);

/*
Increments time and frees threads on the sleepqueue
Will be called in the jiffy irq handler
*/
bool sleepSkipTime(thread_t curThread);

#endif