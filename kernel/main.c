#include <stdint.h>
#include <vga.h>
#include <print.h>
#include <mm/init.h>
#include <param/main.h>
#include <timer.h>
#include <arch.h>

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");
	paramInit();
	mmInit();

	archInit();

	jiffyInit();
	
	sprint("Init complete.\n");
	while(1) {
		asm("hlt");
	}
}

