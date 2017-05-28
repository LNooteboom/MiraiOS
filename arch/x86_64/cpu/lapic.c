#include <arch/cpu.h>

#include <stdint.h>
#include <stdbool.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <print.h>
#include <sched/spinlock.h>
#include <sched/sleep.h>
#include <arch/msr.h>
#include <mm/memset.h>
#include <arch/tlb.h>

struct smpbootInfo {
	uint16_t jump;
	uint16_t nxEnabled;
	uint32_t contAddr;
	uint32_t pml4tAddr;
} __attribute__((packed));


extern char smpboot16start;
extern char smpboot16end;

extern void smpbootStart(void);
extern char VMEM_OFFSET;
extern bool nxEnabled;
extern char PML4T;

uintptr_t physLapicBase;
volatile uint32_t *lapicBase;
size_t cpuInfoSize; //used for AP boot in asm
//void *apExcStacks;

static int getCPUInfo(unsigned int apicID) {
	for (unsigned int i = 0; i < nrofCPUs; i++) {
		if (cpuInfos[i].apicID == apicID) {
			return i;
		}
	}
	return -1;
}

void lapicInit(void) {
	//enable LAPIC
	uint64_t val = rdmsr(0x1B);
	val |= (1 << 11); //set APIC enable bit
	wrmsr(0x1B, val);
	physLapicBase = val & ~0xFFF;
	lapicBase = ioremap(physLapicBase, PAGE_SIZE);
	cpuInfoSize = sizeof(struct cpuInfo);

	uint32_t apicID = lapicBase[0x20 / 4] >> 24;
	int index = getCPUInfo(apicID);
	if (index < 0) {
		sprint("APIC ID is not present\n");
		return;
	}
	struct cpuInfo *info = &(cpuInfos[index]);
	acquireSpinlock(&(info->lock));
	lapicBase[0xF0 / 4] = 0x1FF; //set spurious register to vector 0xFF (points to empty isr)

	tssGdtInit(info);
	wrmsr(0xC0000101, (uint64_t)info); //set GS.base to cpuinfo
	releaseSpinlock(&(info->lock));
}

void ackIRQ(void) {
	lapicBase[0xB0 / 4] = 0;
}

void lapicSendIPI(uint32_t destination, uint8_t vec, enum ipiTypes type) {
	//asm volatile ("cli");
	lapicBase[0x310 / 4] = destination << 24;
	lapicBase[0x300 / 4] = vec | ((type & 7) << 8) | (1 << 14);
	//asm volatile ("sti");
}

void lapicSendIPIToAll(uint8_t vec, enum ipiTypes type) {
	//asm volatile ("cli");
	lapicBase[0x310 / 4] = 0xFF << 24;
	lapicBase[0x300 / 4] = vec | ((type & 7) << 8) | (1 << 14) | (3 << 18);
	//asm volatile ("sti");
}

void lapicDoSMPBoot(void) {
	if (nrofCPUs < 2) {
		return;
	}

	//Prepare smp boot image
	volatile struct smpbootInfo *info = (struct smpbootInfo *)0xFFFFFFFF80070000;
	size_t s = (size_t)(&smpboot16end) - (size_t)(&smpboot16start);
	memcpy(info, &smpboot16start, s);
	info->contAddr = (uint32_t)((uintptr_t)&smpbootStart - (uintptr_t)&VMEM_OFFSET);
	info->pml4tAddr = (uint32_t)((uintptr_t)&PML4T - (uintptr_t)&VMEM_OFFSET);
	info->nxEnabled = nxEnabled;

	//Now boot the APs
	uint32_t currentAPIC = pcpuRead(PCPU_APIC_ID);
	for (unsigned int i = 0; i < nrofCPUs; i++) {
		if (cpuInfos[i].apicID == currentAPIC) {
			continue; //ignore this cpu
		}
		sprint("Starting CPU ");
		decprint(i);
		sprint("...\n");
		
		kthreadSleep(1); //align to milliseconds for maximum timing precision
		lapicSendIPI(cpuInfos[i].apicID, 0, IPI_INIT); //send INIT IPI
		kthreadSleep(10);
		lapicSendIPI(cpuInfos[i].apicID, 0x70, IPI_START); //send SIPI

		//kthreadSleep(10);
		//kthreadCreate(NULL, emptyThread, NULL, THREAD_FLAG_DETACHED); //create empty thread for this cpu
	}
	tlbReloadCR3();
}
