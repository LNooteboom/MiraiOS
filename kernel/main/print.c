#include <print.h>

#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

static int (*stdout)(const char *str) = NULL;

#define PRINTK_STACK_BUF_SZ	512

static void hexprint(char *dest, int len, uint64_t value) {
	for (; len >= 0; len--) {
		char currentNibble = (value >> (len * 4)) & 0x0F;
		if (currentNibble < 10) {
			//0-9
			currentNibble += '0';
		} else {
			currentNibble += 'A' - 10;
		}
		*dest = currentNibble;
		dest++;
	}
}

static int decprint(char *dest, int32_t value) {
	if (value == 0) {
		*dest = '0';
		return 1;
	}

	unsigned int len = 0;
	char buffer[10];
	if (value < 0) {
		value = -value;
		*dest++ = '-';
		len++;
	}
	for (int i = 9; i >= 0; i--) {
		char currentchar = value % 10;
		value /= 10;
		currentchar += '0';
		buffer[i] = currentchar;
	}

	bool zero = true;
	for (int i = 1; i < 10; i++) {
		if (!zero || buffer[i] != '0') {
			*dest++ = buffer[i];
			len++;
			zero = false;
		}
	}
	return len;
}

void vprintk(const char *fmt, va_list args) {
	if (!stdout)
		return;
	char buf[PRINTK_STACK_BUF_SZ];

	unsigned int i = 0;
	for (const char *c = fmt; *c != 0; c++) {
		if (*c != '%') {
			//__putc(*c);
			buf[i++] = *c;
			continue;
		}
		c++;
		switch (*c) {
			case 0:
				return;
			case 'c': {
				char c2 = va_arg(args, int);
				buf[i++] = c2;
				break;
			}
			case 'd': {
				int32_t num = va_arg(args, int32_t);
				i += decprint(&buf[i], num);
				break;
			}
			case 's': {
				char *s = va_arg(args, char *);
				while (*s) {
					buf[i++] = *s;
					s++;
				}
				break;
			}
			case 'x': {
				uint32_t num = va_arg(args, uint32_t);
				hexprint(&buf[i], 7, num);
				i += 8;
				break;
			}
			case 'X': {
				uint64_t num = va_arg(args, uint64_t);
				hexprint(&buf[i], 15, num);
				i += 16;
				break;
			}
			case '%':
				buf[i++] = '%';
				break;
		}
	}
	//releaseSpinlock(&currentConsole->lock);
	va_end(args);
	buf[i] = 0;
	stdout(buf);
}

void printk(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintk(fmt, args);
	va_end(args);
}

void putc(char c) {
	if (!stdout)
		return;
	char buf[2];
	buf[0] = c;
	buf[1] = 0;
	stdout(buf);
}

void puts(const char *text) {
	if (!stdout)
		return;
	stdout(text);
}

void hexprint64(uint64_t value) {
	if (!stdout)
		return;
	//acquireSpinlock(&currentConsole->lock);
	char buf[17];
	unsigned int index = 0;
	for (int i = 15; i >= 0; i--) {
		char currentNibble = (value >> (i * 4)) & 0x0F;
		if (currentNibble < 10) {
			//0-9
			currentNibble += '0';
		} else {
			currentNibble += 'A' - 10;
		}
		//__putc(currentnibble);
		buf[index++] = currentNibble;
	}
	//releaseSpinlock(&currentConsole->lock);
	buf[16] = 0;
	stdout(buf);
}

void hexprintln64(uint64_t value) {
	if (!stdout) return;
	hexprint64(value);
	char buf[2];
	buf[0] = '\n';
	buf[1] = 0;
	stdout(buf);
}

void setKernelStdout(int (*puts)(const char *str)) {
	stdout = puts;
}