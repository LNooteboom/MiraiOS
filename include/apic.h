#ifndef INCLUDE_APIC_H
#define INCLUDE_APIC_H

#include <stdint.h>
#include <spinlock.h>

struct cpuInfo {
	uint32_t apicID;
	spinlock_t lock;
	uint32_t *lapicBase;
};

extern unsigned int nrofCPUs;
extern struct cpuInfo *cpuInfos;

void lapicInit(void);

#endif