#include <global.h>
#include <vga.h>
#include <print.h>
#include <irq.h>
//#include <mm/init.h>
//#include <param/main.h>

//#include <mm/heap.h>
#include <pio.h>

void kmain(void) {
	initInterrupts();
	asm("xchg bx, bx");
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");

	//initParam();
	//initMm();

	while (1) {};
}

