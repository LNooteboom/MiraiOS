#include <panic.h>

#include <stdarg.h>
#include <stddef.h>
#include <sched/smpcall.h>
#include <irq.h>
#include <print.h>

extern void fbPanicUpdate(void);

static __attribute__((noreturn)) void die(void *arg) {
	(void)(arg);
	localInterruptDisable();
	while(true) {
		asm("hlt");
	}
}

void __attribute__((noreturn)) panic(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	smpCallFunction(die, NULL, false);

	vprintk(fmt, args);
	fbPanicUpdate();

	va_end(args);
	
	die(NULL);
}
