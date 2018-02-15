#include <sched/thread.h>
#include <sched/process.h>
#include <stdint.h>
#include <arch/cpu.h>
#include <arch/msr.h>
#include <modules.h>
#include <syscall.h>
#include <errno.h>

#define PRCTL_FS	1
#define PRCTL_GS	2

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



int sysArchPrctl(int which, void *addr) {
	int error = 0;
	uintptr_t uaddr = (uintptr_t)addr;

	if (uaddr & (0xFFFF8000UL << 32)) {
		return -EINVAL;
	}
	thread_t thread = getCurrentThread();
	switch (which) {
		case PRCTL_FS:
			thread->fsBase = addr;
			wrmsr(0xC0000100, uaddr);
			break;
		case PRCTL_GS:
			thread->gsBase = addr;
			wrmsr(0xC0000102, uaddr);
			break;
		default:
			error = -EINVAL;
			break;
	}

	return error;
}

void loadThread(thread_t new) {
	if (new->process) {
		//userspace thread
		if (!new->fsBase) {
			asm ("mov fs, ax" : : "a"(0x23));
		} else {
			wrmsr(0xC0000100, (uint64_t)new->fsBase); //MSR_FS_BASE
		}
		if (!new->gsBase) {
			//asm ("mov gs, ax" : : "a"(0x23));
		} else {
			wrmsr(0xC0000102, (uint64_t)new->gsBase); //MSR_KERNEL_GS_BASE
		}

		asm ("mov cr3, %0" : : "r"(new->process->addressSpace));
	}
	
}