#include <arch/cpu.h>

#include <stdint.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <print.h>
#include <sched/spinlock.h>
#include <sched/sleep.h>
#include <arch/msr.h>
#include <mm/memset.h>

extern char smpboot16start;
extern char smpboot16end;

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
	volatile uint32_t *lapicRegs = ioremap(base, PAGE_SIZE);
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

void lapicSendIPI(uint32_t destination, uint8_t vec, enum ipiTypes type) {
	asm volatile ("cli");
	uint32_t *lapicBase = (uint32_t*)pcpuRead(PCPU_LAPIC_BASE);
	lapicBase[0x310 / 4] = destination << 24;
	lapicBase[0x300 / 4] = vec | ((type & 7) << 8) | (1 << 14);
	asm volatile ("sti");
}

void lapicDoSMPBoot(void) {
	uint32_t currentAPIC = pcpuRead(PCPU_APIC_ID);
	char *brk = (char*)0xFFFFFFFF80070000;
	size_t s = (size_t)(&smpboot16end) - (size_t)(&smpboot16start);
	hexprintln64(s);
	memcpy(brk, &smpboot16start, s);
	for (unsigned int i = 0; i < nrofCPUs; i++) {
		if (cpuInfos[i].apicID == currentAPIC) {
			continue; //ignore this cpu
		}
		kthreadSleep(1); //align to milliseconds for maximum timing precision
		sprint("Starting CPU ");
		decprint(i);
		lapicSendIPI(cpuInfos[i].apicID, 0, IPI_INIT); //send INIT IPI
		kthreadSleep(10);
		asm("xchg bx, bx");
		lapicSendIPI(cpuInfos[i].apicID, 0x70, IPI_START); //send SIPI
	}
}