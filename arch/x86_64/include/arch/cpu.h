#ifndef INCLUDE_ARCH_APIC_H
#define INCLUDE_ARCH_APIC_H

#include <stdint.h>
#include <sched/spinlock.h>
#include <sched/thread.h>
#include <sched/readyqueue.h>
#include <sched/queue.h>

#define pcpuRead64(attr)		doPcpuRead64((uint64_t)(&(( (struct cpuInfo*)0)->attr)))
#define pcpuRead32(attr)		doPcpuRead32((uint64_t)(&(( (struct cpuInfo*)0)->attr)))
#define pcpuWrite64(attr, val)	doPcpuWrite64((uint64_t)(&(( (struct cpuInfo*)0)->attr)), val)
#define pcpuWrite32(attr, val)	doPcpuWrite32((uint64_t)(&(( (struct cpuInfo*)0)->attr)), val)

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

enum ipiTypes {
	IPI_FIXED	= 0,
	IPI_LOWEST	= 1,
	IPI_SMI		= 2,
	IPI_RR		= 3,
	IPI_NMI		= 4,
	IPI_INIT	= 5,
	IPI_START	= 6,
	IPI_EXT		= 7
};

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
	struct cpuInfo *addr;
	thread_t currentThread;
	void *excStackTop;
	uint32_t apicID;
	uint32_t cpuInfosIndex;
	
	spinlock_t lock;

	struct cpuGDTR gdtr;
	gdtEntry_t gdt[16];
	struct cpuTSS tss;

	//scheduling information
	struct threadInfoQueue readyList[NROF_QUEUE_PRIORITIES];
	spinlock_t readyListLock;
	uint32_t threadLoad;
	uint32_t nrofReadyThreads;
};

extern unsigned int nrofCPUs;
extern unsigned int nrofActiveCPUs;
extern struct cpuInfo *cpuInfos;

uint64_t doPcpuRead64(uint64_t addr);
uint32_t doPcpuRead32(uint64_t addr);
void doPcpuWrite64(uint64_t addr, uint64_t value);
void doPcpuWrite32(uint64_t addr, uint32_t value);

void lapicInit(void);
void tssGdtInit(struct cpuInfo *info);

void lapicSendIPI(uint32_t destination, uint8_t vec, enum ipiTypes type);

#endif