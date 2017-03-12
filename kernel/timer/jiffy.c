#include <irq.h>
#include <timer.h>

#include <stdint.h>
#include <stddef.h>
#include <mm/heap.h>
#include <print.h>

uint64_t jiffyCount;
struct jiffyTimer *jiffyTimer;

__attribute__((interrupt)) void jiffyIrq(interruptFrame_t *frame) {
	jiffyCount++;
	hexprintln(jiffyCount);
	hexprintln(0xDEADBEEF);
	while(1);
}

int jiffyInit(void) {
	//select PIT for now
	jiffyTimer = kmalloc(sizeof(struct jiffyTimer));
	i8253Init(jiffyTimer);
	
	routeHWIRQ(jiffyTimer->irq, jiffyIrq, IRQ_FLAG_ISA);
	jiffyTimer->setFreq(1);
	jiffyTimer->setState(true);
	return 0;
}

void jiffyFini(void) {
	kfree(jiffyTimer);
}

