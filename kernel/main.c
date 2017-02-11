#include <global.h>
#include <vga.h>
#include <print.h>
#include <irq.h>
#include <mm/init.h>
#include <param/main.h>

#include <mm/physpaging.h>
#include <mm/paging.h>
#include <mm/heap.h>

bool testEnabled = false;

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");

	paramInit();
	mmInit();
	sprint("Test");
}

