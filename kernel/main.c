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

#include <mm/pagemap.h>

uintptr_t __stack_chk_guard;

thread_t mainThread;

extern void *lapicDoSMPBoot(void *arg);

static void *test(void* arg) {
	//sprint(arg);
	cprint('g');
	//kthreadSleep(10);
}

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");
	paramInit();
	mmInit();
	
	archInit();

	kthreadCreateFromMain(&mainThread);

	jiffyInit();

	thread_t smpThread;
	kthreadCreate(&smpThread, lapicDoSMPBoot, NULL, THREAD_FLAG_RT);
	kthreadJoin(smpThread, NULL);
	thread_t testThread1;
	thread_t testThread2;
	kthreadCreate(&testThread1, test, "12345", THREAD_FLAG_DETACHED);
	kthreadCreate(&testThread2, test, "67890", THREAD_FLAG_DETACHED);
	//hexprintln(testThread1);
	//hexprintln(testThread2);

	sprint("Init complete.\n");
	//while(1);
	kthreadExit(NULL);
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
	sprint("Stack smash detected!");
	while (1) {
		asm("cli");
		asm("hlt");
	}
}
