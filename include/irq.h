#ifndef INCLUDE_IRQ_H
#define INCLUDE_IRQ_H

#include <stdint.h>
#include <stdbool.h>

#define IRQ_HANDLER __attribute__((interrupt))

typedef uint8_t interrupt_t;

typedef uint64_t interruptFrame_t;

extern bool irqEnabled;

void initInterrupts(void);

void routeSoftwareInterrupt(void (*ISR)(void), interrupt_t interrupt);

void routeInterrupt(void (*ISR)(void), interrupt_t interrupt, uint8_t flags);

void unrouteInterrupt(interrupt_t interrupt);

#endif
