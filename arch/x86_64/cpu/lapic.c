#include <arch/cpu.h>

#include <stdint.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <print.h>
#include <spinlock.h>
#include <arch/msr.h>

static int getCPUInfo(unsigned int apicID) {
	for (unsigned int i = 0; i < nrofCPUs; i++) {
		if (cpuInfos[i].apicID == apicID) {
			return i;
		}
	}
	return -1;
}

void lapicInit(void) {
	uint64_t val = rdmsr(0x1B);
	val |= (1 << 11); //set APIC enable bit
	wrmsr(0x1B, val);
	uintptr_t base = val & ~0xFFF;

	uint32_t *lapicRegs = ioremap(base, PAGE_SIZE);
	uint32_t apicID = lapicRegs[0x20 / 4] >> 24;
	int index = getCPUInfo(apicID);
	if (index < 0) {
		sprint("APIC ID is not present\n");
		return;
	}
	struct cpuInfo *info = &(cpuInfos[index]);

	acquireSpinlock(&(info->lock));
	info->lapicBase = lapicRegs;
	lapicRegs[0xF0 / 4] = 0x1FF; //set spurious register to vector 0xFF (points to empty isr)

	tssGdtInit(info);
	wrmsr(0xC0000101, (uint64_t)info);
	releaseSpinlock(&(info->lock));
}

void ackIRQ(void) {
	uint32_t *lapicBase = (uint32_t*)pcpuRead(PCPU_LAPIC_BASE);
	lapicBase[0xB0 / 4] = 0;
}