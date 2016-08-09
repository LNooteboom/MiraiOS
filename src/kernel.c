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
	sprint("Kernel initialising...\n");

	ps2_init();
	PIT_init();
	init_memory_manager();

	void *test = alloc_mem(current_mem_block_table, 10);
	hexprint((int) test);
	newline();
	void *test2 = alloc_mem(current_mem_block_table, 32);
	hexprint((int) test2);
	newline();
	asm("xchgw %bx,%bx");
	dealloc_mem(current_mem_block_table, test);
	//hexprint((int) current_mem_block_table);
	asm("xchgw %bx,%bx");
	dealloc_mem(current_mem_block_table, test2);
	asm("xchgw %bx,%bx");

	sprint("\nInitialisation complete!");
	while (1) {};
}

