#ifndef INCLUDE_SCHED_SMPCALL_H
#define INCLUDE_SCHED_SMPCALL_H

#include <stdbool.h>

void smpCallFunction(void (*func)(void* arg), void *arg, bool wait);

void smpCallInit(void);

#endif