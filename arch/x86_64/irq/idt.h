#ifndef IRQ_IDT_H
#define IRQ_IDT_H

#include <stdint.h>
#include <irq.h>

void initIDT(void);

void routeInterrupt(void (*ISR)(interruptFrame_t *frame), interrupt_t interrupt, uint8_t flags);

void unrouteInterrupt(interrupt_t interrupt);

#endif
