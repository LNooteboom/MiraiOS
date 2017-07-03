#include <stdint.h>
#include <drivers/vga.h>
#include <print.h>
#include <mm/init.h>
#include <param/main.h>
#include <arch.h>
#include <sched/thread.h>
#include <stddef.h>
#include <sched/lock.h>
#include <panic.h>
#include <mm/physpaging.h>

uintptr_t __stack_chk_guard;

thread_t mainThread;

void test(void) {
	sprint("interrupt!\n");
}

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");
	paramInit();
	mmInit();

	kprintf("Detected %dMiB of free memory.\n", getNrofPages() / (1024*1024/PAGE_SIZE) + 16);
	
	earlyArchInit();

	//initialize scheduler
	kthreadCreateFromMain(&mainThread);

	archInit();

	sprint("Init complete.\n");
	kthreadExit(NULL);
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
	panic("Stack smash detected!");
}
