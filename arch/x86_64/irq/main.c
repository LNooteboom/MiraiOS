#include <irq.h>

#include <stdint.h>
#include <mm/heap.h>
#include "idt.h"
#include "exception.h"

bool irqEnabled = 0;

void initInterrupts(void) {
	initIDT();
	initExceptions();
	//irqEnabled = true;
}