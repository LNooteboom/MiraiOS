#include <print.h>

#include <stdint.h>
#include <stdarg.h>
#include <sched/spinlock.h>
#include <console.h>

struct console *currentConsole;

static void __putc(char c) {
	currentConsole->putc(currentConsole, c);
}

static void hexprint(uint32_t value) {
	for (int8_t i = 7; i >= 0; i--) {
		char currentnibble = (value >> (i * 4)) & 0x0F;
		if (currentnibble < 10) {
			//0-9
			currentnibble += '0';
		} else {
			currentnibble += 'A' - 10;
		}
		__putc(currentnibble);
	}
}

static void decprint(int32_t value) {
	if (value == 0) {
		__putc('0');
	}
	char buffer[10];
	if (value < 0) {
		value = -value;
		__putc('-');
	}
	for (int i = 9; i >= 0; i--) {
		char currentchar = value % 10;
		value /= 10;
		currentchar += '0';
		buffer[i] = currentchar;
	}
	char all_zeros = 1;
	for (int i = 1; i < 10; i++) {
		if (!all_zeros || buffer[i] != '0') {
			__putc(buffer[i]);
			all_zeros = 0;
		}
	}
}

int registerConsole(struct console *con) {
	if (con)
		currentConsole = con;
	return 0;
}

void vprintk(const char *fmt, va_list args) {
	if (!currentConsole)
		return;
	acquireSpinlock(&currentConsole->lock);
	for (const char *c = fmt; *c != 0; c++) {
		if (*c != '%') {
			__putc(*c);
			continue;
		}
		c++;
		switch (*c) {
			case 0:
				return;
			case 'c': {
				char c2 = va_arg(args, int);
				__putc(c2);
				break;
			}
			case 'd': {
				int32_t i = va_arg(args, int32_t);
				decprint(i);
				break;
			}
			case 's': {
				char *s = va_arg(args, char *);
				while (*s) {
					__putc(*s);
					s++;
				}
				break;
			}
			case 'x': {
				uint32_t i = va_arg(args, uint32_t);
				hexprint(i);
				break;
			}
			case '%':
				__putc('%');
				break;
		}
	}
	releaseSpinlock(&currentConsole->lock);
	va_end(args);
}

void printk(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintk(fmt, args);
	va_end(args);
}

void putc(char c) {
	if (!currentConsole)
		return;
	acquireSpinlock(&currentConsole->lock);
	__putc(c);
	releaseSpinlock(&currentConsole->lock);
}

void puts(const char *text) {
	if (!currentConsole)
		return;
	acquireSpinlock(&currentConsole->lock);
	while (*text) {
		__putc(*text);
		text++;
	}
	releaseSpinlock(&currentConsole->lock);
}

void hexprint64(uint64_t value) {
	if (!currentConsole)
		return;
	acquireSpinlock(&currentConsole->lock);
	for (int8_t i = 15; i >= 0; i--) {
		char currentnibble = (value >> (i * 4)) & 0x0F;
		if (currentnibble < 10) {
			//0-9
			currentnibble += '0';
		} else {
			currentnibble += 'A' - 10;
		}
		__putc(currentnibble);
	}
	releaseSpinlock(&currentConsole->lock);
}

void hexprintln64(uint64_t value) {
	hexprint64(value);
	__putc('\n');
}