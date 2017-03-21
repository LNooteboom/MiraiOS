#include <stdint.h>
#include <vga.h>
#include <print.h>
#include <irq.h>
#include <mm/init.h>
#include <param/main.h>
#include <acpi.h>
#include <apic.h>
#include <ioapic.h>
#include <timer.h>

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");
	paramInit();
	mmInit();

	acpiInit();
	lapicInit();
	ioApicInit();
	jiffyInit();
	
	sprint("Init complete.\n");
	while (1) {
		asm("hlt");
	}
}

