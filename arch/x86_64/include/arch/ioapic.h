#ifndef INCLUDE_ARCH_IOAPIC_H
#define INCLUDE_ARCH_IOAPIC_H

#include <stdint.h>
#include <spinlock.h>

struct ioApicInfo {
	uint32_t id;
	spinlock_t lock;
	uint32_t gsiBase;
	uint32_t gsiLength;
	volatile uint32_t *indexPort;
	volatile uint32_t *dataPort;
	uintptr_t paddr;
};

extern unsigned int nrofIOApics;
extern struct ioApicInfo *ioApicInfos;

void ioApicInit(void);

#endif