#ifndef INCLUDE_IRQ_H
#define INCLUDE_IRQ_H

#include <global.h>

typedef uint8_t interrupt_t;

void initInterrupts(void);

void routeSoftwareInterrupt(void (*ISR)(void), interrupt_t interrupt);

void routeInterrupt(void (*ISR)(void), interrupt_t interrupt, uint8_t flags);

void unrouteInterrupt(interrupt_t interrupt);

#endif
