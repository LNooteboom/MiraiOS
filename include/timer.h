#ifndef INCLUDE_TIMER_H
#define INCLUDE_TIMER_H

struct jiffyTimer {
	//Sets timer frequency
	int (*setFreq)(unsigned int);
	//turns the timer on
	void (*turnOn)(void);
	//turns the timer off
	void (*turnOff)(void);

	unsigned int minFrequency;
	unsigned int maxFrequency;
	interrupt_t irq;
	uint8_t priority;
};

#endif