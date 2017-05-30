#include <print.h>

#include <stdint.h>
#include <drivers/vga.h>
#include <sched/spinlock.h>

static spinlock_t printLock = 0;

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
