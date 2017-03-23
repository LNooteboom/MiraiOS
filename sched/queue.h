#ifndef QUEUE_H
#define QUEUE_H

#include <sched/thread.h>

struct threadInfo *pullThread(void);

void pushThread(struct threadInfo *thread);

void pushThreadFront(struct threadInfo *thread);

#endif