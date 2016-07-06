#include "tty.h"
#include "video.h"

int linewidth;
int screenheight;
int cursorX;
int cursorY;
char currentattrib = 0x07; //white text on black background

void cprint(char c, char attrib) {
	volatile char *video = (volatile char*)((cursorY * linewidth) + (cursorX * 2) + vram);
	//write_to_vram(c, offset);
	//offset++;
	//write_to_vram(attrib, offset);
	*video++ = c;
	*video = attrib;

	cursorX++;
	if (cursorX >= (linewidth / 2)) {
		newline();
	}
	vga_set_cursor(cursorX, cursorY);
}
void newline(void) {
	cursorX = 0;
	cursorY++;
	//todo: add scrolling
}
void hexprint(int value, char attrib) {
	for (int i = 7; i >= 0; i--) {
		char currentnibble = (value >> (i * 4)) & 0x0F;
		if (currentnibble < 10) {
			//0-9
			currentnibble += '0';
		} else {
			currentnibble += 'A' - 10;
		}
		cprint(currentnibble, attrib);
	}
}
void sprint(char *text, char attrib) {
	while (*text != 0) {
		if (*text == '\n') {
			newline();
			vga_set_cursor(cursorX, cursorY);
		} else {
			cprint(*text, attrib);
		}
		text++;
	}
}

