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

uintptr_t __stack_chk_guard;

thread_t startThread;
thread_t startThread2;

void *testThreadlol(void *arg) {
	while (1) {
		sprint(arg);
		for (int j = 0; j < TIMESLICE_BASE; j++) {
			asm("hlt");
		}
	}
	return NULL;
}

void *testThreadlol2(void *arg) {
	sprint(arg);
	kthreadSleep(2000);
	while (1) {
		sprint(arg);
		for (int j = 0; j < TIMESLICE_BASE; j++) {
			asm("hlt");
		}
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

	thread_t mainThread;
	kthreadCreateFromMain(&mainThread);

	jiffyInit();

	kthreadCreate(&startThread2, testThreadlol2, "B", THREAD_FLAG_FIXED_PRIORITY);
	sprint("Init complete.\n");
	testThreadlol("A");
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