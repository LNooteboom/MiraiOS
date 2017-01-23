#include <global.h>
#include <vga.h>
#include <print.h>
#include <irq.h>
#include <mm/init.h>
#include <param/main.h>

#include <mm/physpaging.h>
#include <mm/paging.h>

uint32_t testcount = 0;

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");

	paramInit();
	mmInit();

	//uint8_t a = 18;
	//uint8_t b = 1;
	//b--;
	//a /= b;

	for (uint16_t i = 0; i < 4096; i++) {
		physPage_t page = allocCleanPhysPage();
		if (i % 256 == 0) {
			hexprintln64(page);
		}
	}
	
	while (1) {};
}

