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
semaphore_t sem;

void *testThreadlol(void *arg) {
	int count = 0;
	while (1) {
		sprint(arg);
		if (count == 2) {
			semWait(&sem);
		} else if (count == 8) {
			semSignal(&sem);
		} else if (count == 10) {
			int ret = 8;
			kthreadJoin(startThread2, (void**)(&ret));
			hexprintln(ret);
			startThread2->state = 0;
		}
		count++;
		asm("hlt");
	}
	return NULL;
}

void *testThreadlol2(void *arg) {
	for(int i = 0; i < 20; i++) {
		sprint(arg);
		if (i == 3) {
			semWait(&sem);
		}
		asm("hlt");
	}
	return (void*)1;
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

	semInit(&sem, 1);
	kthreadCreate(&startThread2, testThreadlol2, "B", THREAD_FLAG_FIXED_PRIORITY);
	sprint("Init complete.\n");
	testThreadlol("A");
	while (1) {
		sprint("C");
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