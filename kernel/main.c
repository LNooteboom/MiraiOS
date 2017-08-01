#include <stdint.h>
#include <print.h>
#include <mm/init.h>
#include <arch.h>
#include <sched/thread.h>
#include <stddef.h>
#include <sched/lock.h>
#include <panic.h>
#include <mm/physpaging.h>
#include <modules.h>
#include <irq.h>

//linker symbols
extern moduleCall_t MODULE_INITS_0_START;
extern moduleCall_t MODULE_INITS_1_START;
extern moduleCall_t MODULE_INITS_2_START;
extern moduleCall_t MODULE_INITS_3_START;
extern moduleCall_t MODULE_INITS_END;

moduleCall_t *moduleInitLevels[NROF_MODULE_INIT_LEVELS + 1] = {
	&MODULE_INITS_0_START,
	&MODULE_INITS_1_START,
	&MODULE_INITS_2_START,
	&MODULE_INITS_3_START,
	&MODULE_INITS_END
};

uintptr_t __stack_chk_guard;

thread_t mainThread;

void kmain(void) {
	initInterrupts();
	mmInit();
	fbInit();
	printk("Detected %dMiB of free memory.\n", getNrofPages() / (1024*1024/PAGE_SIZE) + 16);
	while (1) {
		asm ("hlt" : : "a"(0xEA));
	}
	
	earlyArchInit();


	//initialize scheduler
	kthreadCreateFromMain(&mainThread);

	archInit();

	//execute moduleInits
	for (int level = 0; level < NROF_MODULE_INIT_LEVELS; level++) {
		for (moduleCall_t *i = moduleInitLevels[level]; i < moduleInitLevels[level + 1]; i++) {
			(*i)();
		}
	}
	puts("Init complete.\n");
	kthreadExit(NULL);
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
	panic("Stack smash detected!");
}
