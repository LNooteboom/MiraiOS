#ifndef INCLUDE_IRQ_H
#define INCLUDE_IRQ_H

#include <stdint.h>
#include <stdbool.h>

#define HWIRQ_FLAG_ISA		(1 << 0)
#define HWIRQ_FLAG_CUSTOM	(1 << 1)	//set to 1 for custom polarity or trigger
#define HWIRQ_FLAG_POLARITY	(1 << 13)	//1 for active low, 0 for active high
#define HWIRQ_FLAG_TRIGGER	(1 << 15)	//1 for level, 0 for edge

#define IRQ_FLAG_SHARED		(1 << 0)

typedef uint8_t interrupt_t;

/*
Allocates an interrupt vector
*/
interrupt_t allocIrqVec(void);

/*
Deallocates an interrupt vector
*/
void deallocIrqVec(interrupt_t vec);

/*
Routes an interrupt to a function
*/
int routeInterrupt(void (*handler)(void), interrupt_t vec, unsigned int flags, const char *name);

/*
Unroutes an interrupt
*/
int unrouteInterrupt(interrupt_t vec);

/*
Maps an irq line into the IO-APIC (or similair)
*/
int routeIrqLine(interrupt_t vec, unsigned int irq, unsigned int flags);

/*
Unmaps an irq line
*/
void unrouteIrqLine(unsigned int irq, bool isa);

/*
Disables this cpu's interrupts
*/
static inline void localInterruptDisable(void) {
	asm("cli");
}

/*
Enables this cpu's interrupts
*/
static inline void localInterruptEnable(void) {
	asm("sti");
}

//initialization functions

void initInterrupts(void);

void archInitInterrupts(void);

int addISAOverride(uint32_t dst, uint16_t src, uint16_t flags);

#endif
