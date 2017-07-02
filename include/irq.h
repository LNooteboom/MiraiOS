#ifndef INCLUDE_IRQ_H
#define INCLUDE_IRQ_H

#include <stdint.h>
#include <stdbool.h>

#define IRQ_FLAG_ISA		(1 << 0)
#define IRQ_FLAG_CUSTOM		(1 << 1)	//set to 1 for custom polarity or trigger
#define IRQ_FLAG_POLARITY	(1 << 13)	//1 for active low, 0 for active high
#define IRQ_FLAG_TRIGGER	(1 << 15)	//1 for level, 0 for edge

typedef uint8_t interrupt_t;

void initInterrupts(void);

int addISAOverride(uint32_t dst, uint16_t src, uint16_t flags);

int routeIrqLine(interrupt_t vec, unsigned int irq, unsigned int flags);

void unrouteIrqLine(unsigned int irq, bool isa);

void routeInterrupt(void (*ISR)(void), interrupt_t interrupt, uint8_t flags);

void unrouteInterrupt(interrupt_t interrupt);

interrupt_t allocIrqVec(void);

void deallocIrqVec(interrupt_t vec);

void ackIRQ(void);

static inline void localInterruptDisable(void) {
	asm("cli");
}

static inline void localInterruptEnable(void) {
	asm("sti");
}

#endif
