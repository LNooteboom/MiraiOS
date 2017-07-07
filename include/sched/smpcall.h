#ifndef INCLUDE_SCHED_SMPCALL_H
#define INCLUDE_SCHED_SMPCALL_H

#include <stdbool.h>

/*
Calls a specific function on all other cpu's
if wait is true this function will block until every cpu is finished
*/
void smpCallFunction(void (*func)(void* arg), void *arg, bool wait);

/*
Sets up the smpcall interrupt
*/
void smpCallInit(void);

#endif