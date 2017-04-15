#ifndef INCLUDE_SCHED_SLEEP_H
#define INCLUDE_SCHED_SLEEP_H

#include <sched/thread.h>

void kthreadSleep(unsigned long millis);

bool sleepSkipTime(thread_t curThread);

#endif