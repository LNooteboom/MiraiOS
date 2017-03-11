#include <irq.h>

#include <stdint.h>
#include <mm/heap.h>
#include "idt.h"
#include "exception.h"

#define ISA_LIST_INC	4

struct isaOverride {
	uint16_t source;
	uint16_t flags;
	uint32_t GSI;
};

bool irqEnabled = 0;

static unsigned int isaListLen = 0;
static struct isaOverride *isaOverrides = NULL;

void initInterrupts(void) {
	initIDT();
	initExceptions();
	//irqEnabled = true;
}

int addISAOverride(uint32_t dst, uint16_t src, uint16_t flags) {
	if (isaListLen % ISA_LIST_INC == 0) {
		//alloc more room
		struct isaOverride *oldOverrides = isaOverrides;
		isaOverrides = krealloc(isaOverrides, (isaListLen + ISA_LIST_INC) * sizeof(struct isaOverride));
		if (isaOverrides == NULL) {
			isaOverrides = oldOverrides;
			return -1;
		}
	}
	isaOverrides[isaListLen].source = src;
	isaOverrides[isaListLen].flags = flags;
	isaOverrides[isaListLen].GSI = dst;
	isaListLen++;
	return 0;
}