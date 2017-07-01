#include <panic.h>

#include <stdarg.h>
#include <stddef.h>
#include <sched/smpcall.h>
#include <irq.h>
#include <print.h>

static void die(void *arg) {
	(void)(arg);
	localInterruptDisable();
	asm("hlt");
}

void __attribute__((noreturn)) panic(const char *fmt, ...) {
	va_list args;

	va_start(args, fmt);
	smpCallFunction(die, NULL, false);

	vkprintf(fmt, args);

	va_end(args);
	
	die(NULL);
}
