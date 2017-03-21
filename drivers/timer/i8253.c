#include <drivers/timer/i8253.h>

#include <timer.h>
#include <stdint.h>
#include <pio.h>

#define CH0DATA		0x40
#define CH1DATA		0x41
#define CH2DATA		0x42
#define CMDREG		0x43

#define CMD_ACC_LOWHIGH	(3 << 4)
#define CMD_MODE_COUNT	(0 << 1)
#define CMD_MODE_RATE	(2 << 1)

#define MAXDIV		0x10000

static const uint32_t clkHz = 1193182;
static unsigned int div = 0;

void i8253SetFreq(uint32_t freq) {
	div = (clkHz / freq);
}

void i8253State(bool on) {
	if (on) {
		outb(CMDREG, CMD_ACC_LOWHIGH | CMD_MODE_RATE);
		outb(CH0DATA, div); //low
		outb(CH0DATA, div >> 8); //high
	} else {
		outb(CMDREG, CMD_ACC_LOWHIGH | CMD_MODE_COUNT);
		outb(CH0DATA, 0);
		outb(CH0DATA, 0);
	}
}

void i8253Init(struct jiffyTimer *timerInfo) {
	timerInfo->maxFrequency = clkHz;
	timerInfo->minFrequency = clkHz / MAXDIV;
	timerInfo->irq = 0;
	timerInfo->priority = 0;

	timerInfo->setFreq = i8253SetFreq;
	timerInfo->setState = i8253State;
	i8253State(false);
}

void i8253Fini(void) {
	i8253State(false);
}
