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

void initInterrupts(void);

void archInitInterrupts(void);

int addISAOverride(uint32_t dst, uint16_t src, uint16_t flags);


interrupt_t allocIrqVec(void);

void deallocIrqVec(interrupt_t vec);

int routeInterrupt(void (*handler)(void), interrupt_t vec, unsigned int flags, const char *name);

int unrouteInterrupt(interrupt_t vec);

int routeIrqLine(interrupt_t vec, unsigned int irq, unsigned int flags);

void unrouteIrqLine(unsigned int irq, bool isa);

static inline void localInterruptDisable(void) {
	asm("cli");
}

static inline void localInterruptEnable(void) {
	asm("sti");
}

#endif
