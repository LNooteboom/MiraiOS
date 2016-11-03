#include <print.h>

#include <global.h>
#include <vga.h>

void cprint(char c) {
	vgaCPrint(c);
}
void sprint(char *text) {
	vgaSPrint(text);
}

void hexprint(uint32_t value) {
	for (int i = 7; i >= 0; i--) {
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

void decprint(int32_t value) {
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
