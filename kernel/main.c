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

thread_t startThread;
thread_t startThread2;

void *testThreadlol(void *arg) {
	int count = 0;
	while (1) {
		sprint(arg);
		if (count == 2) {
			//hexprintln(startThread2->returnValue);
			//deallocThread(startThread2);
			int ret = 8;
			joinKernelThread(startThread2, &ret);
			hexprintln(ret);
		}
		count++;
		asm("hlt");
	}
	return NULL;
}

void *testThreadlol2(void *arg) {
	for(int i = 0; i < 20; i++) {
		sprint(arg);
		asm("hlt");
	}
	return 1;
}

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");
	paramInit();
	mmInit();
	
	archInit();

	
	thread_t mainThread;
	createThreadFromMain(&mainThread);
	jiffyInit();

	//createKernelThread(&startThread, testThreadlol, "A");
	createKernelThread(&startThread2, testThreadlol2, "B");
	sprint("Init complete.\n");
	testThreadlol("A");
	while (1) {
		cprint('C');
		//sprint("C");
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