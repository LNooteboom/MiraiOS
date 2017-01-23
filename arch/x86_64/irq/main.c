#include <irq.h>

#include <global.h>
#include "idt.h"
#include "exception.h"

#define MINSWIVALUE 0x30

bool irqEnabled = 0;

void initInterrupts(void) {
	initIDT();
	initExceptions();
	//irqEnabled = true;
}

void routeSoftwareInterrupt(void (*ISR)(void), interrupt_t interrupt) {
	if (interrupt >= MINSWIVALUE) {
		routeInterrupt(ISR, interrupt, 0x01);
	} else {
		//printk("Software interrupt is not valid, ignoring...");
	}
}
