#include <apic.h>
#include "lapic.h"

#include <stdint.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <print.h>
#include <spinlock.h>

unsigned int nrofCPUs = 0;

bool useTSCP;

struct cpuInfo *cpuInfos = NULL;

static int getCPUInfo(unsigned int apicID) {
	for (unsigned int i = 0; i < nrofCPUs; i++) {
		if (cpuInfos[i].apicID == apicID) {
			return i;
		}
	}
	return -1;
}
static void initGDT(struct cpuInfo *info, uint16_t index) {
	info->gdt[0] = 0; 																//NULL entry
	info->gdt[1] = GDT_LONG | GDT_PRESENT | GDT_CODE;								//0x08 64-bit kernel text
	info->gdt[2] = GDT_PRESENT | GDT_DATA;											//0x10 data
	info->gdt[3] = (3UL << GDT_DPL_SHIFT) | GDT_LONG | GDT_PRESENT | GDT_CODE;		//0x18 64-bit usermode text
	info->gdt[4] = (3UL << GDT_DPL_SHIFT) | GDT_OP_SIZE | GDT_PRESENT | GDT_CODE;	//0x20 32-bit usermode text
	info->gdt[7] = index | (0x0000E200UL << 32);									//0x38 fake ldt = cpu index selector
	info->gdt[8] = 0;

	//0x28&30 tss entry
	uintptr_t tssAddr = (uintptr_t)(&(info->tss));
	uint64_t tssSize = sizeof(struct cpuTSS);
	uint32_t tssAddr15 = tssAddr & 0xFFFF;
	uint64_t tssAddr23 = (tssAddr >> 16) & 0xFF;
	uint64_t tssAddr31 = (tssAddr >> 24) & 0xFF;
	uint64_t misc = 0x00E9;
	info->gdt[5] = tssSize | (tssAddr15 << 16) | (tssAddr23 << 32) | (misc << 40) | (tssAddr31 << 56);
	info->gdt[6] = tssAddr >> 32;

	//setup gdtr
	info->gdtr.size = sizeof(gdtEntry_t) * NROF_GDT_ENTRIES;
	info->gdtr.base = (uint64_t)(&(info->gdt));
	asm volatile ("lgdt [%0]" : : "g"(&(info->gdtr)));
	asm volatile ("ltr ax" : : "a"(0x28));
}

void lapicInit(void) {
	uintptr_t base;
	lapicEnable(&base);
	uint32_t *lapicRegs = ioremap(base, PAGE_SIZE);
	uint32_t apicID = lapicRegs[0x20 / 4] >> 24;
	int index = getCPUInfo(apicID);
	if (index < 0) {
		sprint("APIC ID is not present\n");
		return;
	}
	struct cpuInfo *info = &(cpuInfos[index]);
	acquireSpinlock(&(info->lock));

	lapicRegs[0xF0 / 4] = 0x1FF; //set spurious register to vector 0xFF (points to empty isr)
	useTSCP = setTSCAUX((uint32_t)index);
	if (!useTSCP) {
		sprint("No tscp available!\n");
	}
	initGDT(info, index);

	releaseSpinlock(&(info->lock));
}