#include <vga.h>
#include "tty.h"

#include <global.h>
#include "crtc.h"

#define PARSEINT8_LEN 3

uint16_t cursorX = 0;
uint16_t cursorY = 0;
char currentattrib = 0x07; //white text on black background

void vgaCPrint(char c) {
	if (c == '\n') {
		newline();
		return;
	}
	volatile char *video = (volatile char*)((cursorY * screenWidth) + (cursorX * 2) + vram);
	*video++ = c;
	*video = currentattrib;

	cursorX++;
	if (cursorX >= (screenWidth / 2)) {
		newline();
	}
	vgaSetCursor(cursorX, cursorY);
}
void vgaSPrint(char *text) {
	while (*text) {
		switch (*text) {
			case '\e':
				text = execCommand(text + 1);
				break;

			default:
				vgaCPrint(*text);
				break;
		}
		text++;
	}
}

char *execCommand(char *command) {
	switch (*command) {
		case '[':
			command++;
			char *commandLetter = command;
			while (*commandLetter < 64 || *commandLetter > 126) {
				commandLetter++;
			}

			switch (*commandLetter) {
				case 'J':
					switch (*command) {
						case '0':
							//clear up
							break;
						case '1':
							//clear down
							break;
						case '2':
							//clear screen
							clearScreen();
							break;
					}
					break;

				case 'm':
					do {
						uint8_t number0 = (*command++ - '0');
						uint8_t number1;
						if (*command >= '0' && *command <= '9') {
							number1 = *command - '0';
							command += 2;
						} else {
							number1 = number0;
							number0 = 0;
							command += 1;
						}
						switch (number0) {
							case 0:
								//misc
								switch (number1) {
									case 0:
										//reset
										currentattrib = 0x07;
									case 1:
										currentattrib |= 0x08;
										break;
									case 2:
										currentattrib &= 0x77;
										break;
								}
								break;

							case 3:
								//foreground color
								currentattrib = (currentattrib & 0xF8) | number1;
								break;

							case 4:
								//background color
								currentattrib = (currentattrib & 0x8F) | (number1 << 4);
								break;
						}
					} while (*(command - 1) == ';');
			}

			return commandLetter;
	}
	return command;
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
	for (uint16_t y = 0; y < screenHeight; y++) {
		for (uint16_t x = 0; x < screenWidth / 2; x++) {
			//volatile char *video = (volatile char*)((y * screenWidth) + (x * 2) + vram + 1);
			*video = attrib;
			video += 2;
		}
	}
}
void clearScreen(void) {
	volatile char *video = vram;
	for (uint16_t y = 0; y < screenHeight; y++) {
		for (uint16_t x = 0; x < screenWidth / 2; x++) {
			//volatile char *video = (volatile char*)((y * screenWidth) + (x * 2) + vram);
			*video++ = 0;
			*video++ = currentattrib;
		}
	}
	cursorX = 0;
	cursorY = 0;
	vgaSetCursor(cursorX, cursorY);
}
