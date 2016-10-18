#include <global.h>
#include <vga.h>
#include <print.h>
#include <irq.h>

void kmain(void) {
	initInterrupts();
	initVga();
	sprint("\e0m\e[2JKernel initialising...\n");

	uint8_t *ptr = (uint8_t*) 0x80000000;
	uint8_t a = *ptr;

	while (1) {};
}

