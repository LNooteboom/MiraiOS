#include <irq.h>

#include <global.h>
#include "idt.h"

#define MINSWIVALUE 0x30

void initInterrupts(void) {
	initIDT();
	//TODO: map exceptions in IDT
}

void routeSoftwareInterrupt(void (*ISR)(void), interrupt_t interrupt) {
	if (interrupt >= MINSWIVALUE) {
		routeInterrupt(ISR, interrupt, 0x01);
	} else {
		//printk("Software interrupt is not valid, ignoring...");
	}
}
