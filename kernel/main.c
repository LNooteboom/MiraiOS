#include <video.h>
#include <irq.h>

void kmain(void) {
	initInterrupts();
	initVga();
	sprint("\033Kernel initialising...\n");

	uint8_t var = 1;
	cprint(3 / (var - 1));

	while (1) {};
}

