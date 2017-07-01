#include <stdint.h>
#include <drivers/vga.h>
#include <print.h>
#include <mm/init.h>
#include <param/main.h>
#include <timer.h>
#include <arch.h>
#include <sched/thread.h>
#include <stddef.h>
#include <sched/lock.h>
#include <panic.h>
#include <mm/physpaging.h>

uintptr_t __stack_chk_guard;

thread_t mainThread;

extern void *lapicDoSMPBoot(void *arg);

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");
	paramInit();
	mmInit();

	sprint("Detected ");
	decprint(getNrofPages() / (1024*1024/PAGE_SIZE) + 16);
	sprint("MiB of free memory.\n");
	
	archInit();

	kthreadCreateFromMain(&mainThread);

	jiffyInit();

	thread_t smpThread;
	kthreadCreate(&smpThread, lapicDoSMPBoot, NULL, THREAD_FLAG_RT);
	kthreadJoin(smpThread, NULL);

	sprint("Init complete.\n");
	kthreadExit(NULL);
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
	panic("Stack smash detected!");
}
