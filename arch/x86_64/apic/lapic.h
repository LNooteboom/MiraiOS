#ifndef LAPIC_H
#define LAPIC_H

#include <stdint.h>
#include <stdbool.h>

void lapicEnable(uintptr_t *baseAddr);

bool setTSCAUX(uint32_t value);

#endif