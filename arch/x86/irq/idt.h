#ifndef IRQ_IDT_H
#define IRQ_IDT_H

#include <global.h>
#include <irq.h>

void initIDT(void);

void routeInterrupt(void (*ISR)(void), interrupt_t interrupt, uint8_t flags);

void unrouteInterrupt(interrupt_t interrupt);

#endif
