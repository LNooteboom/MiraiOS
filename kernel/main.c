#include <global.h>
#include <vga.h>
#include <print.h>
#include <irq.h>
#include <mm/init.h>
#include <param/main.h>

#include <mm/physpaging.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <mm/lowmem.h>

bool testEnabled = false;

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");
	paramInit();
	mmInit();
	asm("xchg bx, bx");
	hexprintln64(allocLowPhysPages(1));
	hexprintln64(allocLowPhysPages(16));
	hexprintln64(allocLowPhysPages(1));
	hexprintln64(allocLowPhysPages(1));
	hexprintln64(allocLowPhysPages(1));
	while(true);
}

