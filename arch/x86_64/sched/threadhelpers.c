#include <sched/thread.h>

#include <stdint.h>
#include <arch/cpu.h>

thread_t getCurrentThread(void) {
	return (thread_t)pcpuRead64(currentThread);
}

void setCurrentThread(thread_t thread) {
	pcpuWrite64(currentThread, (uint64_t)thread);
}

uint32_t getCPUThreadLoad(void) {
	return pcpuRead32(threadLoad);
}

void setCPUThreadLoad(uint32_t load) {
	pcpuWrite32(threadLoad, load);
}