#ifndef INCLUDE_DRIVERS_TIMER_I8253_H
#define INCLUDE_DRIVERS_TIMER_I8253_H

#include <stdint.h>
#include <stdbool.h>

void i8253SetFreq(uint32_t freq);

void i8253State(bool on);

#endif