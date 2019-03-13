#include <irq.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sched/spinlock.h>
#include <errno.h>

#define IRQDESC_START			32
#define IRQDESC_NROF_ENTRIES	(128 - 32)

static spinlock_t irqBitmapLock;
static uint32_t irqBitmap[IRQDESC_NROF_ENTRIES / 32];

struct IrqDescriptor {
	void (*handler)(void *);
	void *context;
	const char *name;
	unsigned int flags;

	unsigned int irqCount;
};

struct IrqDescriptor irqDescs[IRQDESC_NROF_ENTRIES];

interrupt_t allocIrqVec(void) {
	int bitPos;
	int arrayPos;
	bool foundVec = false;
	acquireSpinlock(&irqBitmapLock);
	for (unsigned int i = 0; i < IRQDESC_NROF_ENTRIES / 32; i++) {
		if (irqBitmap[i] != (uint32_t)(~0)) {
			bitPos = 0;
			uint32_t bit = irqBitmap[i];
			while (bit & 1) {
				bitPos++;
				bit >>= 1;
			}
			irqBitmap[i] |= (1 << bitPos);
			arrayPos = i;
			foundVec = true;
			break;
		}
	}
	releaseSpinlock(&irqBitmapLock);
	if (!foundVec) {
		return 0;
	}
	return (arrayPos * 32) + bitPos + IRQDESC_START;
}
void deallocIrqVec(interrupt_t vec) {
	vec -= IRQDESC_START;
	int bitPos = vec % 32;
	int arrayPos = vec / 32;
	acquireSpinlock(&irqBitmapLock);
	irqBitmap[arrayPos] &= ~(1 << bitPos);
	releaseSpinlock(&irqBitmapLock);
}

int routeInterrupt(void (*handler)(void *), void *context, interrupt_t vec, unsigned int flags, const char *name) {
	vec -= IRQDESC_START;
	if (vec >= IRQDESC_NROF_ENTRIES) {
		return -EINVAL;
	}
	if (irqDescs[vec].handler) {
		//already in use
		return -EBUSY;
	}
	//map it in the bitmap
	int bitPos = vec % 32;
	int arrayPos = vec / 32;
	acquireSpinlock(&irqBitmapLock);
	irqBitmap[arrayPos] |= (1 << bitPos);
	releaseSpinlock(&irqBitmapLock);

	irqDescs[vec].handler = handler;
	irqDescs[vec].name = name;
	irqDescs[vec].flags = flags;
	irqDescs[vec].irqCount = 0;
	return 0;
}

int unrouteInterrupt(interrupt_t vec) {
	vec -= IRQDESC_START;
	if (vec >= IRQDESC_NROF_ENTRIES) {
		return -EINVAL;
	}
	irqDescs[vec].handler = NULL;
	return 0;
}

void handleIRQ(interrupt_t vec) {
	vec -= IRQDESC_START;
	if (vec >= IRQDESC_NROF_ENTRIES || irqDescs[vec].handler == NULL) {
		return;
	}
	irqDescs[vec].handler(irqDescs[vec].context);
	irqDescs[vec].irqCount++;
}

void initInterrupts(void) {
	archInitInterrupts();
}

