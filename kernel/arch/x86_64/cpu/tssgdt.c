#include <arch/cpu.h>

#include <stdint.h>
#include <stddef.h>

void tssGdtInit(struct CpuInfo *info) {
	//gdt descriptors need to be layed out in a specfic way to work with syscall/sysret
	info->gdt[0] = 0; 																//NULL entry
	info->gdt[1] = GDT_LONG | GDT_PRESENT | GDT_CODE;								//0x08 64-bit kernel text
	info->gdt[2] = GDT_PRESENT | GDT_DATA | GDT_WRITE;								//0x10 kernel data
	info->gdt[3] = (3UL << GDT_DPL_SHIFT) | GDT_OP_SIZE | GDT_PRESENT | GDT_CODE;	//0x18 32-bit usermode text
	info->gdt[4] = (3UL << GDT_DPL_SHIFT) | GDT_PRESENT | GDT_DATA | GDT_WRITE;		//0x20 usermode data
	info->gdt[5] = (3UL << GDT_DPL_SHIFT) | GDT_LONG | GDT_PRESENT | GDT_CODE;		//0x28 64-bit usermode text
	

	//0x30&38 tss entry
	uintptr_t tssAddr = (uintptr_t)(&(info->tss));
	uint64_t tssSize = sizeof(struct CpuTSS);
	uint32_t tssAddr15 = tssAddr & 0xFFFF;
	uint64_t tssAddr23 = (tssAddr >> 16) & 0xFF;
	uint64_t tssAddr31 = (tssAddr >> 24) & 0xFF;
	uint64_t misc = 0x00E9;
	info->gdt[6] = tssSize | (tssAddr15 << 16) | (tssAddr23 << 32) | (misc << 40) | (tssAddr31 << 56);
	info->gdt[7] = tssAddr >> 32;

	//setup gdtr
	info->gdtr.size = sizeof(gdtEntry_t) * NROF_GDT_ENTRIES;
	info->gdtr.base = (uint64_t)(&(info->gdt));

	//setup ist entry
	info->tss.ist[0] = info->excStackTop;
	//set IST for page fault, NMI and MCE
	idtSetIST(14, 1);
	idtSetIST(2, 1);
	idtSetIST(18, 1);

	asm volatile ("lgdt [%0]" : : "g"(&(info->gdtr)));
	asm volatile ("ltr ax" : : "a"(0x30));
}

/*void tssSetIST(int ist, void *addr) {
	struct CpuInfo *cpuInfoNull = NULL;
	doPcpuWrite64((uint64_t)(&cpuInfoNull->tss.ist[ist - 1]), (uint64_t)addr);
}*/

void tssSetRSP0(void *rsp) {
	struct CpuInfo *cpuInfoNull = NULL;
	doPcpuWrite64((uint64_t)(&cpuInfoNull->tss.rsp[0]), (uint64_t)rsp);
}