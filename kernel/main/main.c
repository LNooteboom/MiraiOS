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
#include <fs/fs.h>

//linker symbols
extern moduleCall_t MODULE_INITS_0_START;
extern moduleCall_t MODULE_INITS_1_START;
extern moduleCall_t MODULE_INITS_2_START;
extern moduleCall_t MODULE_INITS_3_START;
extern moduleCall_t MODULE_INITS_END;

extern int createInitProcess(void);

moduleCall_t *moduleInitLevels[NROF_MODULE_LEVELS + 1] = {
	&MODULE_INITS_0_START,
	&MODULE_INITS_1_START,
	&MODULE_INITS_2_START,
	&MODULE_INITS_3_START,
	&MODULE_INITS_END
};

uintptr_t __stack_chk_guard;

thread_t mainThread;

static void executeModuleCallLevel(unsigned int level) {
	for (moduleCall_t *i = moduleInitLevels[level]; i < moduleInitLevels[level + 1]; i++) {
		int error = (*i)();
		if (error) {
			puts("[MAIN] moduleCall at ");
			hexprint64((uint64_t)i);
			printk(" returned non-zero value: %d\n", error);
		}
	}
}

void kmain(void) {
	initInterrupts();
	mmInit();
	executeModuleCallLevel(0);
	printk("[MM] Detected %dMiB of free memory\n", getNrofPages() / (1024*1024/PAGE_SIZE) + 16);
	
	earlyArchInit();

	//initialize scheduler
	kthreadCreateFromMain(&mainThread);
	archInit();

	executeModuleCallLevel(1);

	//initialize fs
	executeModuleCallLevel(2);
	
	//create /dev directory
	fsCreate(NULL, rootDir, "dev", ITYPE_DIR);

	//initialize drivers
	executeModuleCallLevel(3);

	puts("[MAIN] Initialization complete\n");

	int error = execInit("init");
	printk("%d\n", error);

	kthreadExit(NULL);
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
	panic("Stack smash detected!");
}
