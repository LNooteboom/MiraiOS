#ifndef INCLUDE_PIO_H
#define INCLUDE_PIO_H

#include <global.h>

typedef uint16_t ioport_t;


uint8_t inb(ioport_t port);

void outb(ioport_t port, uint8_t value);

#endif
