#include <arch/cpu.h>

#include "lapicdefs.h"
#include <stdint.h>
#include <stdbool.h>
#include <arch/msr.h>
#include <arch/tlb.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <sched/spinlock.h>
#include <sched/sleep.h>
#include <print.h>
#include <io.h>
#include <arch/map.h>

#define PIT_HZ				1193182

#define PIT_CH2DATA			0x42
#define PIT_CMDREG			0x43
#define PIT_CMD_ACC_LOWHIGH	(3 << 4)
#define PIT_CMD_MODE_COUNT	(0 << 1)
#define PIT_CMD_MODE_RATE	(2 << 1)

struct SmpbootInfo {
	uint16_t jump;
	uint16_t nxEnabled;
	uint32_t pml4tAddr;
	void *contAddr;
	
} __attribute__((packed));


extern char smpboot16start;
extern char smpboot16end;

extern void smpbootStart(void);
extern char VMEM_OFFSET;
extern bool nxEnabled;

extern bool perCpuTimer;

uintptr_t physLapicBase;
volatile char *lapicBase;
size_t cpuInfoSize; //used for AP boot in asm
volatile bool cpuStartedUp;

int32_t busSpeed;

static int getCPUInfo(unsigned int apicID) {
	for (unsigned int i = 0; i < nrofCPUs; i++) {
		if (cpuInfos[i].apicID == apicID) {
			return i;
		}
	}
	return -1;
}

static int32_t getBusSpeed(void) {
	//set lapic timer divide to 16
	write32(lapicBase + LAPIC_DIV, 3);
	//set oneshot + interrupt to dummy irq (0xE0)
	write32(lapicBase + LAPIC_LVT_TMR, 0xE0);

	//pit chnl 2 to speaker enable(bit 0 set), speaker data disabled(bit 1 clear)
	out8(0x61, (in8(0x61) & 0xFD) | 1);
	//set channel 2 to hw one-shot, access low/high
	out8(PIT_CMDREG, (2 << 6) | (3 << 4) | (1 << 1));
	//10ms
	int pitVal = PIT_HZ / 100;
	out8(PIT_CH2DATA, pitVal);
	out8(PIT_CH2DATA, pitVal >> 8);
	//start pit
	char data = in8(0x61);
	out8(0x61, data & ~1);
	out8(0x61, data | 1);

	//start lapic counter
	write32(lapicBase + LAPIC_TIC, ~0);
	//wait for spkr gate to go high again (read bit 5 from 0x61)
	
	while (!(in8(0x61) & (1 << 5))) {
		//asm ("pause");
	}
	//stop lapic timer
	write32(lapicBase + LAPIC_LVT_TMR, LAPIC_MASK);
	int32_t counterVal = 0 - (int32_t)read32(lapicBase + LAPIC_TCC);
	return counterVal * 1600; //multiply by 16 (divider) and 100 (we only ran for 10ms)
}

void lapicInit(void) {
	//enable LAPIC
	uint64_t addr = rdmsr(0x1B);
	physLapicBase = addr & ~0xFFF;
	lapicBase = ioremap(physLapicBase, PAGE_SIZE);
	cpuInfoSize = sizeof(struct CpuInfo);

	uint32_t apicID = read32(lapicBase + LAPIC_ID) >> 24;
	int index = getCPUInfo(apicID);
	if (index < 0) {
		puts("APIC ID is not present\n");
		return;
	}
	write32(lapicBase + 0xE0, 0x0FFFFFFFF);
	write32(lapicBase + 0xD0, (read32(lapicBase + 0xD0) & 0x00FFFFFF) | 1);
	write32(lapicBase + LAPIC_LVT_PERF, LAPIC_MASK);
	write32(lapicBase + LAPIC_LVT_LINT0, LAPIC_MASK);
	write32(lapicBase + LAPIC_LVT_LINT1, LAPIC_MASK);
	write32(lapicBase + LAPIC_LVT_TMR, LAPIC_MASK);
	write32(lapicBase + LAPIC_TPR, 0);

	addr |= (1 << 11); //set APIC enable bit
	wrmsr(0x1B, addr);
	write32(lapicBase + LAPIC_SPURIOUS, 0x1FF); //set spurious register to vector 0xFF (points to empty isr)

	busSpeed = getBusSpeed();
	printk("[LAPIC] Bus speed: %dHz\n", busSpeed / 100);

	if (busSpeed / 100 >= 100000) {
		perCpuTimer = true;
	}

	struct CpuInfo *info = &(cpuInfos[index]);
	acquireSpinlock(&(info->lock));
	info->active = true;
	tssGdtInit(info);
	wrmsr(0xC0000101, (uint64_t)info); //set GS.base to cpuinfo

	releaseSpinlock(&(info->lock));
	
}

void lapicEnableTimer(interrupt_t vec) {
	write32(lapicBase + LAPIC_LVT_TMR, vec | (1 << 17)); //set vector + periodic mode
	write32(lapicBase + LAPIC_DIV, 3); //set divider to 16
	write32(lapicBase + LAPIC_TIC, busSpeed / (16 * JIFFY_HZ));
	printk("[LAPIC] Timer enabled\n");
}

void ackIRQ(void) {
	write32(lapicBase + LAPIC_EOI, 0);
}

void lapicSendIPI(uint32_t destination, uint8_t vec, enum ipiTypes type) {
	write32(lapicBase + LAPIC_ICRH, destination << 24);
	write32(lapicBase + LAPIC_ICRL, vec | ((type & 7) << 8) | (1 << 14));
}

void lapicSendIPIToAll(uint8_t vec, enum ipiTypes type) { //TODO test & fix this
	write32(lapicBase + LAPIC_ICRH, 0xFF << 24);
	write32(lapicBase + LAPIC_ICRL, vec | ((type & 7) << 8) | (1 << 14) | (3 << 18));
}

void lapicDoSMPBoot(void) {
	if (nrofCPUs < 2) {
		return;
	}

	//Get cr3
	uint64_t cr3;
	asm ("mov rax, cr3" : "=a"(cr3));

	//Prepare smp boot image
	volatile struct SmpbootInfo *info = (struct SmpbootInfo *)0xFFFFFFFF80070000;
	size_t s = (size_t)(&smpboot16end) - (size_t)(&smpboot16start);
	memcpy(info, &smpboot16start, s);
	info->contAddr = smpbootStart;
	info->pml4tAddr = cr3;
	info->nxEnabled = nxEnabled;

	//Now boot the APs
	uint32_t currentAPIC = pcpuRead32(apicID);
	for (unsigned int i = 0; i < nrofCPUs; i++) {
		if (cpuInfos[i].apicID == currentAPIC) {
			continue; //ignore this cpu
		}
		printk("[SMPBOOT] Starting CPU %d...  ", i);

		cpuStartedUp = false;
		
		kthreadSleep(1); //align to milliseconds for maximum timing precision
		lapicSendIPI(cpuInfos[i].apicID, 0, IPI_INIT); //send INIT IPI
		kthreadSleep(10);
		lapicSendIPI(cpuInfos[i].apicID, 0x70, IPI_START); //send SIPI

		//while (!cpuStartedUp) {
		volatile bool *active = &cpuInfos[i].active;
		while (!*active) {
			asm("pause");
		}
		printk("[OK]\n");
	}
	mmUnmapBootPages();
	tlbReloadCR3();
}
