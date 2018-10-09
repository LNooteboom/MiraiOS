#include <sched/readyqueue.h>

#include <arch/cpu.h>
#include <sched/queue.h>
#include <sched/thread.h>
#include <stdint.h>
#include <stddef.h>

//static struct ThreadInfoQueue readyList[NROF_QUEUE_PRIORITIES];
//spinlock_t readyListLock;

static void calcNewPriority(thread_t thread) {
	int timeslice = TIMESLICE_BASE << thread->priority;
	int remaining = thread->jiffiesRemaining;
	if (thread->fixedPriority || (remaining > 0 && remaining < (timeslice / 2) )) {
		//if thread used up more than half of its timeslice, but wasn't preempted
		//Same priority
		//Do nothing
	} else if (remaining < 1) {
		//If thread was preempted
		//Lower priority (increment the priority value)
		if (thread->priority < NROF_QUEUE_PRIORITIES - 1) {
			thread->priority += 1;
		}
	} else if (remaining >= (timeslice / 2)) {
		//If thread used up less than half of its timeslice
		//Higher priority
		if (thread->priority > 1) {
			thread->priority -= 1;
		}
	}
}

static thread_t getThreadFromCPU(struct CpuInfo *cpu) {
	thread_t ret = NULL;
	for (int i = 0; i < NROF_QUEUE_PRIORITIES; i++) {
		struct ThreadQueueEntry *qe = threadQueuePop(&cpu->readyList[i]);
		if (qe) {
			ret = qe->thread;
			cpu->nrofReadyThreads--;
			ret->jiffiesRemaining = TIMESLICE_BASE << i;
			break;
		}
	}
	return ret;
}

thread_t readyQueuePop(void) {
	//try this cpu first
	struct CpuInfo *thisCPU = (struct CpuInfo*)pcpuRead64(addr);
	acquireSpinlock(&thisCPU->readyListLock);
	thread_t ret = getThreadFromCPU(thisCPU);
	releaseSpinlock(&thisCPU->readyListLock);
	if (ret || nrofActiveCPUs < 2) {
		//releaseSpinlock(&thisCPU->readyListLock);
		return ret;
	}

	struct CpuInfo *busiestCPU = NULL;
	uint32_t load = 0;
	for (unsigned int i = 0; i < nrofCPUs; i++) {
		if (&cpuInfos[i] == thisCPU) {
			continue;
		}
		acquireSpinlock(&cpuInfos[i].readyListLock);
		//cprint('a');
		if (cpuInfos[i].nrofReadyThreads > 0 && cpuInfos[i].threadLoad > load) {
			busiestCPU = &cpuInfos[i];
			load = cpuInfos[i].threadLoad;
		}
		releaseSpinlock(&cpuInfos[i].readyListLock);
	}
	if (busiestCPU) {
		acquireSpinlock(&busiestCPU->readyListLock);
		ret = getThreadFromCPU(busiestCPU);
		if (ret) {
			acquireSpinlock(&ret->lock);
			ret->cpuAffinity = thisCPU->cpuInfosIndex;
			busiestCPU->threadLoad -= NROF_QUEUE_PRIORITIES - ret->priority;
			acquireSpinlock(&thisCPU->readyListLock);
			thisCPU->threadLoad += NROF_QUEUE_PRIORITIES - ret->priority;
			releaseSpinlock(&thisCPU->readyListLock);
			releaseSpinlock(&ret->lock);
		}
		releaseSpinlock(&busiestCPU->readyListLock);
		return ret;
	}
	return NULL;
}

void readyQueuePush(struct ThreadQueueEntry *qe) {
	thread_t thread = qe->thread;
	if (qe->sigDepth < thread->sigDepth) {
		qe->sigCont = true;
		return;
	}

	thread->state = THREADSTATE_SCHEDWAIT;
	calcNewPriority(thread);
	int priority = thread->priority;
	uint32_t cpuIndex = thread->cpuAffinity;

	if (cpuInfos[thread->cpuAffinity].threadLoad) {
		for (unsigned int i = 0; i < nrofCPUs; i++) {
			if (cpuInfos[i].active && cpuInfos[i].threadLoad == 0) {
				cpuIndex = i;
				thread->cpuAffinity = i;
				break;
			}
		}
	}

	acquireSpinlock(&cpuInfos[cpuIndex].readyListLock);
	cpuInfos[cpuIndex].threadLoad += NROF_QUEUE_PRIORITIES - priority;
	cpuInfos[cpuIndex].nrofReadyThreads++;
	threadQueuePush(&cpuInfos[cpuIndex].readyList[priority], thread);
	releaseSpinlock(&cpuInfos[cpuIndex].readyListLock);

	if (pcpuRead32(cpuInfosIndex) != cpuIndex) {
		lapicSendIPI(cpuInfos[cpuIndex].apicID, RESCHED_VEC, IPI_FIXED);
	}
}

thread_t readyQueueExchange(thread_t thread, bool front) {
	struct CpuInfo *thisCPU = (struct CpuInfo*)pcpuRead64(addr);
	if (front) {
		acquireSpinlock(&thisCPU->readyListLock);
		threadQueuePushFront(&thisCPU->readyList[thread->priority], thread);
	} else {
		int oldPriority = thread->priority;
		calcNewPriority(thread);
		acquireSpinlock(&thisCPU->readyListLock);
		thisCPU->threadLoad += thread->priority - oldPriority;
		threadQueuePush(&thisCPU->readyList[thread->priority], thread);
	}
	thisCPU->nrofReadyThreads++;
	thread_t ret = getThreadFromCPU(thisCPU);
	releaseSpinlock(&thisCPU->readyListLock);
	return ret;
}

