#ifndef INCLUDE_PIO_H
#define INCLUDE_PIO_H

#include <stdint.h>

typedef uint16_t ioport_t;

static inline uint8_t inb(uint16_t port) {
	uint8_t data;
	asm("in al, dx" : "=a" (data) : "d" (port));
	return data;
}

static inline void outb(uint16_t port, uint8_t data) {
	asm("out dx, al" : : "a" (data), "d" (port));
}

#endif
