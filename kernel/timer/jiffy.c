#include <irq.h>
#include <timer.h>

#include <stdint.h>
#include <stddef.h>

uint64_t jiffyCount;
struct timer *jiffyTimer;

int jiffyInit(void) {
	//select PIT for now
	jiffyTimer = vmalloc(sizeof(struct jiffyTimer));
	i8253Init(jiffyTimer);
	
	jiffyTimer->setFreq(1);
	jiffyTimer->setState(true);
}

void jiffyFini(void) {
	kfree(jiffyTimer);
}

IRQ_HANDLER void jiffyIrq(interruptFrame_t *frame) {
	jiffyCount++;
	hexprint(jiffyCount);
}