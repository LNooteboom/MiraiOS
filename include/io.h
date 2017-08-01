#ifndef INCLUDE_PIO_H
#define INCLUDE_PIO_H

#include <stdint.h>

/*
Reads a 32-bit value from a specified address in memory
*/
static inline uint32_t read32(volatile void *addr) {
	uint64_t data;
	asm volatile ("mov %0, [%1]" : "=r" (data) : "g" (addr));
	return data;
}

/*
Writes a 32-bit value to a specified address in memory
*/
static inline void write32(volatile void *addr, uint32_t data) {
	uint64_t data64 = data;
	asm volatile ("mov [%0], %1" : : "g" (addr), "r" (data64));
}

/*
Reads an 8-bit value from an io-port
*/
static inline uint8_t in8(uint16_t port) {
	uint8_t data;
	asm volatile ("in al, dx" : "=a" (data) : "d" (port));
	return data;
}

/*
Writes an 8-bit value to an io-port
*/
static inline void out8(uint16_t port, uint8_t data) {
	asm volatile ("out dx, al" : : "a" (data), "d" (port));
}

#endif
