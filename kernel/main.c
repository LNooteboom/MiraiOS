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

void *test(__attribute__((unused)) void *arg) {
	sprint("lol");
	return NULL;
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
	kthreadCreate(NULL, test, NULL, 0);
	kthreadCreate(NULL, test, NULL, 0);

	sprint("Init complete.\n");
	kthreadExit(NULL);
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
	sprint("Stack smash detected!");
	while (1) {
		asm("cli");
		asm("hlt");
	}
}
