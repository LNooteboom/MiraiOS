#ifndef INCLUDE_IRQ_H
#define INCLUDE_IRQ_H

#include <stdint.h>
#include <stdbool.h>

#define IRQ_HANDLER __attribute__((interrupt))

typedef uint8_t interrupt_t;

typedef uint64_t interruptFrame_t;

extern bool irqEnabled;

void initInterrupts(void);

int addISAOverride(uint32_t dst, uint16_t src, uint16_t flags);

void routeInterrupt(void (*ISR)(interruptFrame_t *frame), interrupt_t interrupt, uint8_t flags);

void unrouteInterrupt(interrupt_t interrupt);

#endif
