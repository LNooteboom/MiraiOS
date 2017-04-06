#include <sched/thread.h>

#include <stdint.h>
#include <apic.h>

thread_t getCurrentThread(void) {
	//This is unsafe, it should only be used in an interrupt context
	return cpuInfos[getCPU()].currentThread;
}

void setCurrentThread(thread_t thread) {
	cpuInfos[getCPU()].currentThread = thread;
}