#ifndef IRQ_IDT_H
#define IRQ_IDT_H

#include <stdint.h>
#include <irq.h>

/*
Initialize the interrupt descriptor table
*/
void initIDT(void);

/*
Map an interrupt in the IDT
*/
void mapIdtEntry(void (*ISR)(void), interrupt_t interrupt, uint8_t flags);

/*
Unmap an interrupt in the IDT
*/
void unmapIdtEntry(interrupt_t interrupt);

/*
Set an IDT entry to use the IST mechanism
*/
void idtSetIST(interrupt_t interrupt, int ist);

/*
Set whether an interrupt is accessible by userspace INT instruction
*/
void idtSetDPL(interrupt_t interrupt, bool user);

#endif
