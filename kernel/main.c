#include <global.h>
#include <vga.h>
#include <print.h>
#include <irq.h>
#include <mm/init.h>
#include <param/main.h>

#include <mm/heap.h>

void kmain(void) {
	initInterrupts();
	initVga();
	sprint("\e[0m\e[2JKernel initialising...\n");

	initParam();
	initMm();

	void *vp = vmalloc(0x2000);
	hexprintln(vp);
	void *vp2 = vmalloc(0x10);
	hexprintln(vp2);
	kfree(vp);
	vp = vmalloc(0x20);
	hexprintln(vp);

	while (1) {};
}

