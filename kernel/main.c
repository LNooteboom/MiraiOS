#include <global.h>
#include <vga.h>
#include <print.h>
#include <irq.h>
#include <mm/init.h>
#include <param/main.h>

void kmain(void) {
	initInterrupts();
	initVga();
	sprint("\e[0m\e[2JKernel initialising...\n");

	initParam();
	initMm();
	//uint8_t *ptr = (uint8_t*) 0x80000000;
	//uint8_t a = *ptr;

	while (1) {};
}

