#include "io.h"
#include "video.h"
#include "kernel.h"
#include "memory.h"
#include "param.h"
#include "irq.h"
#include "tty.h"
#include "pit.h"
#include "ps2.h"

void kmain(void) {
	init_memory();
	video_init();
	irq_init();
	initPICS();
	cursorX = partable->cursorX;
	cursorY = partable->cursorY;
	tty_set_full_screen_attrib(0x07);
	sprint("Kernel initialising...\n", currentattrib);

	ps2_init();
	PIT_init();
	page_stack_setup();

	newline();
	int page = alloc_page();
	hexprint(page, currentattrib);
	newline();
	int page2 = alloc_page();
	hexprint(page2, currentattrib);
	newline();
	dealloc_page(page);
	page = alloc_page();
	hexprint(page, currentattrib);

	sprint("\nPurification complete!", currentattrib);
	while (1) {};
}

