//#pragma once

#include "io.h"
#include "video.h"
#include "kernel.h"
#include "memory.h"
#include "param.h"
#include "irq.h"

int linewidth;
int cursorX;
int cursorY;

void kmain(void) {
	init_memory();
	video_init();
	irq_init();
	linewidth = get_line_width();
	cursorX = partable->cursorX;
	cursorY = partable->cursorY;
	//do nothing for now
	while (1) {};
}

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
void hexprint(int value) {
	for (int i = 7; i >= 0; i--) {
		char currentnibble = (value >> (i * 4)) & 0x0F;
		if (currentnibble < 10) {
			//0-9
			currentnibble += '0';
		} else {
			currentnibble += 'A' - 10;
		}
		cprint(currentnibble, 0x07);
	}
}
