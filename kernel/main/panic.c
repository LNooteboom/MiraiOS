#include <panic.h>

#include <stdarg.h>
#include <stddef.h>
#include <sched/smpcall.h>
#include <irq.h>
#include <print.h>
#include <mm/paging.h>
#include <mm/pagemap.h>

extern void fbPanicUpdate(void);

static __attribute__((noreturn)) void die(void *arg) {
	(void)(arg);
	localInterruptDisable();
	while(true) {
		asm("hlt");
	}
}

static void stackTrace(uintptr_t rsp) {
	if (!mmGetPageEntry(rsp)) {
		return;
	}
	printk("Stack trace:\n");
	for (int i = 0; i < 32; i++) {
		if (rsp % PAGE_SIZE == 0) {
			break;
		}
		printk("%X: %X\n", rsp, *((uint64_t *)rsp));
		rsp += 8;
	}
}

void __attribute__((noreturn)) panic(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	smpCallFunction(die, NULL, false);

	uintptr_t rsp;
	asm("mov rax, rsp" : "=a"(rsp));
	stackTrace(rsp);


	vprintk(fmt, args);
	fbPanicUpdate();

	va_end(args);
	
	die(NULL);
}
