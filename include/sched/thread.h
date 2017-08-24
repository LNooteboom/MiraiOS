#ifndef INCLUDE_SCHED_THREAD_H
#define INCLUDE_SCHED_THREAD_H

#include <stdint.h>
#include <stdbool.h>
#include <sched/spinlock.h>

#define JIFFY_HZ	1000

#define THREAD_FLAG_DETACHED		1
#define THREAD_FLAG_FIXED_PRIORITY	2
#define THREAD_FLAG_RT				4

#define TIMESLICE_BASE	(JIFFY_HZ / 10)

enum threadState {
	THREADSTATE_FINISHED,
	THREADSTATE_RUNNING,
	THREADSTATE_SCHEDWAIT,
	THREADSTATE_LOCKWAIT,
	THREADSTATE_SLEEP
};

struct threadInfo {
	//asm accessible part
	uintptr_t stackPointer;		//0x00
	void *returnValue;			//0x08
	enum threadState state;		//0x10
	spinlock_t lock;			//0x14
	bool detached;				//0x18
	uint32_t cpuAffinity;		//0x1C
	//end of asm accessible part

	struct threadInfo *nextThread;
	struct threadInfo *prevThread;

	int priority;
	bool fixedPriority;
	int jiffiesRemaining;

	unsigned long sleepTime;
	
	struct threadInfo *joinFirst;
	struct threadInfo *joinLast;
	int nrofJoinThreads;
};

typedef struct threadInfo *thread_t;

extern unsigned long jiffyCounter;

/*
Creates a new kernel thread
*/
int kthreadCreate(thread_t *thread, void *(*start)(void *), void *arg, int flags);

/*
Registers the main function as a thread
*/
int kthreadCreateFromMain(thread_t *thread);

/*
Joins a kernel thread
*/
void kthreadJoin(thread_t thread, void **returnValue);

/*
Detaches a kernel thread
*/
void kthreadDetach(void);

thread_t getCurrentThread(void);
void setCurrentThread(thread_t thread);

uint32_t getCPUThreadLoad(void);
void setCPUThreadLoad(uint32_t load);

/*
Exit the current thread, freeing any joined threads
*/
void kthreadExit(void *ret);

/*
Puts the current thread to sleep
*/
void kthreadStop(void);

/*
Deallocates a thread from memory
*/
void deallocThread(thread_t thread);

#endif