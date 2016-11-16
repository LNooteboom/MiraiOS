#include <global.h>
#include <vga.h>
#include <print.h>
#include <irq.h>
#include <mm/init.h>
#include <param/main.h>

#include <mm/heap.h>
#include <mm/physpaging.h>

void kmain(void) {
	initInterrupts();
	initVga();
	sprint("\e[0m\e[2JKernel initialising...\n");

	initParam();
	initMm();
	//uint8_t *ptr = (uint8_t*) 0x80000000;
	//uint8_t a = *ptr;
	uintptr_t p = kmalloc(0x20);
	hexprintln(p);
	uintptr_t p2 = kmalloc(0x08);
	hexprintln(p2);
	kfree(p);
	uintptr_t p3 = kmalloc(0x10);
	hexprintln(p3);

	uintptr_t vp = vmalloc(0x1800);
	hexprintln(vp);

	while (1) {};
}

