#ifndef INCLUDE_ARCH_APIC_H
#define INCLUDE_ARCH_APIC_H

#include <stdint.h>
#include <sched/spinlock.h>
#include <sched/thread.h>

//pcpu Addresses
#define PCPU_THREAD		0
#define PCPU_LAPIC_BASE	8
#define PCPU_APIC_ID	16

#define NROF_GDT_ENTRIES 9

//gdt selector bits
#define GDT_WRITE		(1UL << 41)
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
	thread_t currentThread;
	uint32_t *lapicBase;
	uint64_t apicID;
	
	spinlock_t lock;

	struct cpuGDTR gdtr;
	gdtEntry_t gdt[16];
	struct cpuTSS tss;
};

extern unsigned int nrofCPUs;
extern struct cpuInfo *cpuInfos;

/*
static inline uint64_t pcpuRead(uint64_t addr) {
	uint64_t ret;
	asm volatile ("mov rax, [gs:rdi]" : "=a"(ret) : "D"(addr));
	return ret;
}

static inline void pcpuWrite(uint64_t addr, uint64_t value) {
	asm volatile ("mov [gs:rdi], rax" : : "D"(addr), "a"(value));
}
*/
uint64_t pcpuRead(uint64_t addr);
void pcpuWrite(uint64_t addr, uint64_t value);

void lapicInit(void);
void tssGdtInit(struct cpuInfo *info);

#endif