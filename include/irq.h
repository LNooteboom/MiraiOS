#ifndef INCLUDE_IRQ_H
#define INCLUDE_IRQ_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t interrupt_t;

extern bool irqEnabled;

void initInterrupts(void);

void routeSoftwareInterrupt(void (*ISR)(void), interrupt_t interrupt);

void routeInterrupt(void (*ISR)(void), interrupt_t interrupt, uint8_t flags);

void unrouteInterrupt(interrupt_t interrupt);

#endif
