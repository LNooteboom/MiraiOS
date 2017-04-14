#include <sched/thread.h>

#include <stdint.h>
#include <arch/cpu.h>

thread_t getCurrentThread(void) {
	return (thread_t)pcpuRead(PCPU_THREAD);
}

void setCurrentThread(thread_t thread) {
	pcpuWrite(PCPU_THREAD, (uint64_t)thread);
}