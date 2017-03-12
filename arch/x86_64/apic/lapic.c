#include <apic.h>
#include "lapic.h"

#include <stdint.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <print.h>
#include <spinlock.h>

unsigned int nrofCPUs = 0;

struct cpuInfo *cpuInfos = NULL;

static struct cpuInfo *getCPUInfo(unsigned int apicID) {
	for (unsigned int i = 0; i < nrofCPUs; i++) {
		if (cpuInfos[i].apicID == apicID) {
			return &(cpuInfos[i]);
		}
	}
	return NULL;
}

void lapicInit(void) {
	uintptr_t base;
	lapicEnable(&base);
	if (!cpuInfos) {
		cpuInfos = kmalloc(nrofCPUs * sizeof(struct cpuInfo));
	}
	uint32_t *lapicRegs = ioremap(base, PAGE_SIZE);
	uint32_t apicID = lapicRegs[0x20 / 4] >> 24;
	struct cpuInfo *info = getCPUInfo(apicID);
	if (!info) {
		sprint("APIC ID is not present\n");
		return;
	}
	acquireSpinlock(&(info->lock));

	lapicRegs[0xF0 / 4] = 0x1FF; //set spurious register to vector 0xFF (points to empty isr)

	releaseSpinlock(&(info->lock));
}