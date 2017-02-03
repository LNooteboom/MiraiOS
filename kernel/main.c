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
	
	char *test = vmalloc(32);
	hexprintln64(test);
	char *test2 = vmalloc(32);
	hexprintln64(test2);
	kfree(test);
	char *test3 = vmalloc(16);
	hexprintln64(test3);
}

