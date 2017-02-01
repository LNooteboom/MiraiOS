#include <global.h>
#include <vga.h>
#include <print.h>
#include <irq.h>
#include <mm/init.h>
#include <param/main.h>

#include <mm/physpaging.h>
#include <mm/paging.h>

bool testEnabled = false;

void kmain(void) {
	initInterrupts();
	vgaInit();
	sprint("\e[0m\e[2JKernel initialising...\n");

	paramInit();
	mmInit();

	for (uint16_t i = 0; i < 4096; i++) {
		physPage_t page = allocCleanPhysPage();
		if (i % 256 == 0) {
			hexprintln64(page);
			//cprint('-');
		}
	}
	
}

