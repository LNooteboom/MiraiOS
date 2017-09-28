#ifndef INCLUDE_ARCH_APIC_H
#define INCLUDE_ARCH_APIC_H

#include <stdint.h>
#include <stdbool.h>
#include <sched/spinlock.h>
#include <sched/thread.h>
#include <sched/readyqueue.h>
#include <sched/queue.h>
#include <irq.h>

#define JIFFY_VEC	0xC2
#define RESCHED_VEC	0xC3

#define pcpuRead64(attr)		doPcpuRead64((uint64_t)(&(( (struct CpuInfo*)0)->attr)))
#define pcpuRead32(attr)		doPcpuRead32((uint64_t)(&(( (struct CpuInfo*)0)->attr)))
#define pcpuWrite64(attr, val)	doPcpuWrite64((uint64_t)(&(( (struct CpuInfo*)0)->attr)), val)
#define pcpuWrite32(attr, val)	doPcpuWrite32((uint64_t)(&(( (struct CpuInfo*)0)->attr)), val)

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

struct CpuTSS {
	uint32_t reserved0;
	uint64_t rsp[3];
	uint64_t reserved1;
	uint64_t ist[7];
	uint64_t reserved2;
	uint16_t reserved3;
	uint16_t ioMapBase;
} __attribute__((packed));
struct CpuGDTR {
	uint16_t size;
	uint64_t base;
} __attribute__((packed));

struct CpuInfo {
	//asm accessable
	struct CpuInfo *addr;	//00
	thread_t currentThread;	//08
	void *excStackTop;		//10
	uint32_t apicID;		//18
	uint32_t cpuInfosIndex;	//1C
	bool active;			//20
	spinlock_t lock;		//24

	struct CpuGDTR gdtr;
	gdtEntry_t gdt[16];
	struct CpuTSS tss;

	//scheduling information
	struct ThreadInfoQueue readyList[NROF_QUEUE_PRIORITIES];
	spinlock_t readyListLock;
	uint32_t threadLoad;
	uint32_t nrofReadyThreads;
};

extern unsigned int nrofCPUs;
extern unsigned int nrofActiveCPUs;
extern struct CpuInfo *cpuInfos;

uint64_t doPcpuRead64(uint64_t addr);
uint32_t doPcpuRead32(uint64_t addr);
void doPcpuWrite64(uint64_t addr, uint64_t value);
void doPcpuWrite32(uint64_t addr, uint32_t value);

void lapicInit(void);
void lapicEnableTimer(interrupt_t vec);
void tssGdtInit(struct CpuInfo *info);
void lapicDoSMPBoot(void);
void lapicSendIPI(uint32_t destination, uint8_t vec, enum ipiTypes type);

void ackIRQ(void);

void tssSetRSP0(void *rsp);

#endif