#include <irq.h>
#include <timer.h>

#include <stdint.h>
#include <stddef.h>
#include <mm/heap.h>
#include <print.h>
#include <drivers/timer/i8253.h>

extern void jiffyIrq(void);

uint64_t jiffyCounter;
struct jiffyTimer *jiffyTimer;
int jiffyVec;

/*
void jiffyIrq(interruptFrame_t *frame) {
	jiffyCount++;
	hexprintln(jiffyCount);
	hexprintln(0xDEADBEEF);
	while(1);
}*/

int jiffyInit(void) {
	//select PIT for now
	jiffyTimer = kmalloc(sizeof(struct jiffyTimer));
	i8253Init(jiffyTimer);
	
	jiffyVec = routeHWIRQ(jiffyTimer->irq, jiffyIrq, IRQ_FLAG_ISA);
	jiffyTimer->setFreq(JIFFY_HZ);
	jiffyTimer->setState(true);
	return 0;
}

void jiffyFini(void) {
	kfree(jiffyTimer);
}

