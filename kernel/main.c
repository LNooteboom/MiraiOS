#include <stdint.h>
#include <vga.h>
#include <print.h>
#include <mm/init.h>
#include <param/main.h>
#include <timer.h>
#include <arch.h>
#include <sched/thread.h>
#include <stddef.h>

uintptr_t __stack_chk_guard;

void *testThreadlol(void *arg) {
	while (1) {
		sprint(arg);
		asm("hlt");
	}
	return NULL;
}

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");
	paramInit();
	mmInit();
	
	archInit();

	jiffyInit();
	thread_t mainThread;
	createThreadFromMain(&mainThread);
	
	thread_t startThread;
	thread_t startThread2;
	createKernelThread(&startThread, testThreadlol, "A");
	createKernelThread(&startThread2, testThreadlol, "B");
	sprint("Init complete.\n");
	testThreadlol("C");
	while (1) {
		asm("hlt");
	}
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
	sprint("Stack smash detected!");
	while (1) {
		asm("cli");
		asm("hlt");
	}
}