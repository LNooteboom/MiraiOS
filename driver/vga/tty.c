#include <vga.h>
#include "tty.h"

#include <global.h>
#include "crtc.h"
#include <print.h>

char currentAttrib = 0x07; //white text on black background
uint16_t cursor = 0;
uint16_t scroll = 0;

void vgaCPrint(char c) {
	if (c == '\n') {
		newline();
		return;
	}
	vgaMem[cursor * 2] = c;
	vgaMem[cursor * 2 + 1] = currentAttrib;

	cursor++;
	vgaSetCursor(cursor);
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
										currentAttrib = 0x07;
										break;
									case 1:
										currentAttrib |= 0x08;
										break;
									case 2:
										currentAttrib &= 0x77;
										break;
								}
								break;

							case 3:
								//foreground color
								currentAttrib = (currentAttrib & 0xF8) | number1;
								break;

							case 4:
								//background color
								currentAttrib = (currentAttrib & 0x8F) | (number1 << 4);
								break;
						}
					} while (*(command - 1) == ';');
			}
			return commandLetter;
	}
	return command;
}

void newline(void) {
	//uint8_t cursorY = cursor / vgaScreenWidth + 1;
	//cursor = cursorY * vgaScreenWidth;
	cursor -= (cursor % vgaScreenWidth); //go to beginning of line (cr)
	cursor += vgaScreenWidth; //go to next line (lf)
	vgaSetCursor(cursor);
}
void backspace(void) {
	vgaMem[cursor * 2 - 2] = 0;
	vgaMem[cursor * 2 - 1] = currentAttrib;
	cursor--;
	cursorLeft();
}
void cursorLeft(void) {
	while (vgaMem[cursor * 2] == 0 && cursor) {
		cursor--;
	}
}

void setFullScreenColor(char attrib) {
}
void clearScreen(void) {
}
