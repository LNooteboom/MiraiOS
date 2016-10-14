#include "tty.h"

#include <global.h>
#include "vga.h"

uint8_t cursorX = 0;
uint16_t cursorY = 0;
char currentattrib = 0x07; //white text on black background

void cprint(char c) {
	volatile char *video = (volatile char*)((cursorY * screenWidth) + (cursorX * 2) + vram);
	*video++ = c;
	*video = currentattrib;

	cursorX++;
	/*if (cursorX >= (screenWidth / 2)) {
		newline();
	}*/
	vgaSetCursor(cursorX, cursorY);
}
void sprint(char *text) {
	while (*text != 0) {
		if (*text == '\n') {
			newline();
			vgaSetCursor(cursorX, cursorY);
		} else {
			cprint(*text);
		}
		text++;
	}
}

void newline(void) {
	cursorX = 0;
	cursorY++;
	if (cursorY >= screenHeight) {
		vgaSetScroll(++scrollY);
	}
	vgaSetCursor(cursorX, cursorY);
}
void backspace(void) {
	volatile char *video = (volatile char*)((cursorY * screenWidth) + (cursorX * 2) + vram);
	video -= 2;
	if (cursorX == 0) {
		if (cursorY == 0) {
			return; //cursor at start of the screen
		}
		cursorY--;
		cursorX = (screenWidth / 2);
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
	vgaSetCursor(cursorX, cursorY);
}
void cursorLeft(void) {
	volatile char *video = (volatile char*)((cursorY * screenWidth) + (cursorX * 2) + vram);
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
	for (int y = 0; y < screenHeight; y++) {
		for (int x = 0; x < screenWidth / 2; x++) {
			//volatile char *video = (volatile char*)((y * screenWidth) + (x * 2) + vram + 1);
			*video = attrib;
			video += 2;
		}
	}
}
void clearScreen(void) {
	volatile char *video = vram;
	for (int y = 0; y < screenHeight; y++) {
		for (int x = 0; x < screenWidth / 2; x++) {
			//volatile char *video = (volatile char*)((y * screenWidth) + (x * 2) + vram);
			*video = 0;
			video += 2;
		}
	}
	cursorX = 0;
	cursorY = 0;
	vgaSetCursor(cursorX, cursorY);
}
