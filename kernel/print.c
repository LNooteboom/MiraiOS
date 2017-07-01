#include <print.h>

#include <stdint.h>
#include <stdarg.h>
#include <drivers/vga.h>
#include <sched/spinlock.h>

static spinlock_t printLock = 0;

void vkprintf(const char *fmt, va_list args) {
	for (const char *c = fmt; *c != 0; c++) {
		if (*c != '%') {
			cprint(*c);
			continue;
		}
		c++;
		switch (*c) {
			case 0:
				return;
			case 'c': {
				char c2 = va_arg(args, int);
				cprint(c2);
				break;
			}
			case 'd': {
				int32_t i = va_arg(args, int32_t);
				decprint(i);
				break;
			}
			case 's': {
				char *s = va_arg(args, char *);
				sprint(s);
				break;
			}
			case 'x': {
				uint32_t i = va_arg(args, uint32_t);
				hexprint(i);
				break;
			}
			case '%':
				cprint('%');
				break;
		}
	}
	va_end(args);
}

void kprintf(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vkprintf(fmt, args);
	va_end(args);
}

void cprint(char c) {
	acquireSpinlock(&printLock);
	vgaCPrint(c);
	releaseSpinlock(&printLock);
}
void sprint(const char *text) {
	acquireSpinlock(&printLock);
	vgaSPrint(text);
	releaseSpinlock(&printLock);
}

void hexprint(uint32_t value) {
	for (int8_t i = 7; i >= 0; i--) {
		char currentnibble = (value >> (i * 4)) & 0x0F;
		if (currentnibble < 10) {
			//0-9
			currentnibble += '0';
		} else {
			currentnibble += 'A' - 10;
		}
		cprint(currentnibble);
	}
}

void hexprintln(uint32_t value) {
	hexprint(value);
	cprint('\n');
}

void hexprint64(uint64_t value) {
	for (int8_t i = 15; i >= 0; i--) {
		char currentnibble = (value >> (i * 4)) & 0x0F;
		if (currentnibble < 10) {
			//0-9
			currentnibble += '0';
		} else {
			currentnibble += 'A' - 10;
		}
		cprint(currentnibble);
	}
}
void hexprintln64(uint64_t value) {
	hexprint64(value);
	cprint('\n');
}

void decprint(int32_t value) {
	if (value == 0) {
		cprint('0');
	}
	char buffer[10];
	if (value < 0) {
		value = -value;
		cprint('-');
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
			cprint(buffer[i]);
			all_zeros = 0;
		}
	}
}
void decprintln(int32_t value) {
	decprint(value);
	cprint('\n');
}
