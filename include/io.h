#ifndef INCLUDE_PIO_H
#define INCLUDE_PIO_H

#include <stdint.h>

static inline uint32_t read32(volatile void *addr) {
	uint32_t data;
	asm volatile ("mov %0, [%1]" : "=r" (data) : "g" (addr));
	return data;
}

static inline void write32(volatile void *addr, uint32_t data) {
	asm volatile ("mov [%0], %1" : : "g" (addr), "r" (data));
}

static inline uint8_t in8(uint16_t port) {
	uint8_t data;
	asm volatile ("in al, dx" : "=a" (data) : "d" (port));
	return data;
}

static inline void out8(uint16_t port, uint8_t data) {
	asm volatile ("out dx, al" : : "a" (data), "d" (port));
}

#endif
