#include "tty.h"
#include "video.h"

int cursorX;
int cursorY;
char currentattrib = 0x07; //white text on black background

void cprint(char c) {
	volatile char *video = (volatile char*)((cursorY * screenwidth) + (cursorX * 2) + vram);
	*video++ = c;
	*video = currentattrib;

	cursorX++;
	if (cursorX >= (screenwidth / 2)) {
		newline();
	}
	vga_set_cursor(cursorX, cursorY);
}
void sprint(char *text) {
	while (*text != 0) {
		if (*text == '\n') {
			newline();
			vga_set_cursor(cursorX, cursorY);
		} else {
			cprint(*text);
		}
		text++;
	}
}

void newline(void) {
	cursorX = 0;
	cursorY++;
	if (cursorY >= screenheight) {
		vga_set_scroll(++scrollY);
	}
	vga_set_cursor(cursorX, cursorY);
}
void backspace(void) {
	volatile char *video = (volatile char*)((cursorY * screenwidth) + (cursorX * 2) + vram);
	video -= 2;
	if (cursorX == 0) {
		if (cursorY == 0) {
			return; //cursor at start of the screen
		}
		cursorY--;
		cursorX = (screenwidth / 2);
		while (*video == 0) {
			if (cursorX == 0) {
				break;
			}
			cursorX--;
			video -= 2;
		}
	} else {
		*video = 0;
		cursorX--;
	}
	vga_set_cursor(cursorX, cursorY);
}
void cursorLeft(void) {
	volatile char *video = (volatile char*)((cursorY * screenwidth) + (cursorX * 2) + vram);
	video -= 2;
	while (*video == 0) {
		if (cursorX == 0) {
			break;
		}
		cursorX--;
		video -= 2;
	}
}

void setFullScreenColor(char attrib) {
	currentattrib = attrib;
	volatile char *video = vram + 1;
	for (int y = 0; y < screenheight; y++) {
		for (int x = 0; x < screenwidth / 2; x++) {
			//volatile char *video = (volatile char*)((y * screenwidth) + (x * 2) + vram + 1);
			*video = attrib;
			video += 2;
		}
	}
}
void clearScreen(void) {
	volatile char *video = vram;
	for (int y = 0; y < screenheight; y++) {
		for (int x = 0; x < screenwidth / 2; x++) {
			//volatile char *video = (volatile char*)((y * screenwidth) + (x * 2) + vram);
			*video = 0;
			video += 2;
		}
	}
	cursorX = 0;
	cursorY = 0;
	vga_set_cursor(cursorX, cursorY);
}
