#ifndef INCLUDE_TIMER_H
#define INCLUDE_TIMER_H

#include <stdint.h>
#include <irq.h>
#include <stdbool.h>

struct jiffyTimer {
	//Sets timer frequency
	void (*setFreq)(uint32_t);
	//turns the timer on/off
	void (*setState)(bool);

	unsigned int minFrequency;
	unsigned int maxFrequency;
	interrupt_t irq;
	uint8_t priority;
};

int jiffyInit(void);

void jiffyFini(void);

#endif