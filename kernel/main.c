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
#include <sched/lock.h>
#include <mm/physpaging.h>

uintptr_t __stack_chk_guard;

thread_t mainThread;

semaphore_t testSem;

extern void *lapicDoSMPBoot(void *arg);

void *test(void *arg) {
	//asm("xchg bx, bx");
	//sprint(arg);
	semSignal(&testSem);
	return NULL;
}

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

	semInit(&testSem, -19999);
	hexprintln64(getNrofPages());

	for (int i = 0; i < 20000; i++) {
		kthreadCreate(NULL, test, (void*)i, THREAD_FLAG_DETACHED);
	}
	thread_t test2Thread;
	kthreadCreate(&test2Thread, test, NULL, 0);
	kthreadJoin(test2Thread, NULL);

	semWait(&testSem);
	sprint("Init complete.\n");
	hexprintln64(getNrofPages());
	kthreadExit(NULL);
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
	sprint("Stack smash detected!");
	while (1) {
		asm("cli");
		asm("hlt");
	}
}
