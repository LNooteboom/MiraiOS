#ifndef INCLUDE_DRIVERS_TIMER_I8253_H
#define INCLUDE_DRIVERS_TIMER_I8253_H

#include <timer.h>

void i8253Init(struct jiffyTimer *timerInfo);

void i8253Fini(void);

#endif