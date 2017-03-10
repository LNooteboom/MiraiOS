#ifndef INCLUDE_TIMER_H
#define INCLUDE_TIMER_H

#include <stdint.h>

struct jiffyTimer {
	//Sets timer frequency
	int (*setFreq)(uint32_t);
	//turns the timer on/off
	void (*setState)(bool);

	unsigned int minFrequency;
	unsigned int maxFrequency;
	interrupt_t irq;
	uint8_t priority;
};

#endif