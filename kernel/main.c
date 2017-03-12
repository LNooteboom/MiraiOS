#include <stdint.h>
#include <vga.h>
#include <print.h>
#include <irq.h>
#include <mm/init.h>
#include <param/main.h>
#include <mm/heap.h>
#include <acpi.h>
#include <apic.h>

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");
	paramInit();
	mmInit();

	acpiInit();
	lapicInit();
	
	sprint("Init complete.\n");
}

