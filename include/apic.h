#ifndef INCLUDE_APIC_H
#define INCLUDE_APIC_H

#include <stdint.h>
#include <spinlock.h>

#define NROF_GDT_ENTRIES 9

//gdt selector bits
#define GDT_CONFORMING	(1UL << 42)
#define GDT_DPL_SHIFT	45
#define GDT_PRESENT		(1UL << 47)
#define GDT_LONG		(1UL << 53)
#define GDT_OP_SIZE		(1UL << 54)
#define GDT_DATA		(1UL << 44)
#define GDT_CODE		((1UL << 43) | (1UL << 44))

typedef uint64_t gdtEntry_t;

struct cpuTSS {
	uint32_t reserved0;
	uint64_t rsp[3];
	uint64_t reserved1;
	uint64_t ist[7];
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t ioMapBase;
};
struct cpuGDTR {
	uint16_t size;
	uint64_t base;
} __attribute__((packed));

struct cpuInfo {
	uint32_t apicID;
	spinlock_t lock;
	uint32_t *lapicBase;

	struct cpuGDTR gdtr;
	gdtEntry_t gdt[8];
	struct cpuTSS tss;
};

extern unsigned int nrofCPUs;
extern struct cpuInfo *cpuInfos;

uint32_t getCPU(void);

void lapicInit(void);

#endif