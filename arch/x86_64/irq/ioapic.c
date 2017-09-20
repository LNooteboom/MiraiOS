#include <arch/ioapic.h>
#include <irq.h>

#include <stdint.h>
#include <stddef.h>
#include <sched/spinlock.h>
#include <mm/paging.h>
#include <mm/heap.h>
#include <print.h>
#include <arch/cpu.h>

#define ISA_LIST_INC	4

#define REG_ID			0x00
#define REG_VERSION		0x01
#define REG_ARB			0x02
#define REG_IORED_BASE	0x10

#define IORED_APIC_ID_SHIFT	56
#define IORED_FLAG_MASK		(1 << 16)
#define IORED_DELMODE_SHIFT 8

struct IsaOverride {
	uint16_t source;
	uint16_t flags;
	uint32_t GSI;
};

static unsigned int isaListLen = 0;
static struct IsaOverride *isaOverrides = NULL;

unsigned int nrofIOApics = 0;
struct IoApicInfo *ioApicInfos = NULL;

static interrupt_t gsiToVec[256];

static struct IoApicInfo *findApic(unsigned int irq) {
	for (unsigned int i = 0; i < nrofIOApics; i++) {
		if (irq >= ioApicInfos[i].gsiBase && irq < ioApicInfos[i].gsiBase + ioApicInfos[i].gsiLength) {
			return &(ioApicInfos[i]);
		}
	}
	return NULL;
}

void ioApicInit(void) {
	for (unsigned int i = 0; i < nrofIOApics; i++) {
		acquireSpinlock(&(ioApicInfos[i].lock));
		ioApicInfos[i].indexPort = ioremap(ioApicInfos[i].paddr, 0x20);
		ioApicInfos[i].dataPort = (uint32_t*)((uintptr_t)(ioApicInfos[i].indexPort) + 0x10);
		//get gsiLength
		*(ioApicInfos[i].indexPort) = REG_VERSION;
		uint32_t ver = *(ioApicInfos[i].dataPort);
		//hexprintln(ver);
		ioApicInfos[i].gsiLength = (ver >> 16) & 0xFF;
		releaseSpinlock(&(ioApicInfos[i].lock));
	}
	printk("[IOAPIC] Initialized\n");
	localInterruptEnable();
}

int routeIrqLine(interrupt_t vec, unsigned int irq, unsigned int flags) {
	if (flags & HWIRQ_FLAG_ISA) {
		unsigned int isaFlags = 0;
		bool found = false;
		for (unsigned int i = 0; i < isaListLen; i++) {
			if (isaOverrides[i].source == irq) {
				irq = isaOverrides[i].GSI;
				isaFlags = isaOverrides[i].flags;
				found = true;
				break;
			}
		}
		if (found) {
			flags = HWIRQ_FLAG_CUSTOM;
			if ((isaFlags & 3) != 1) {
				flags |= HWIRQ_FLAG_POLARITY;
			}
			if (((isaFlags >> 2) & 3) == 3) {
				flags |= HWIRQ_FLAG_TRIGGER;
			}
		} else {
			flags = 0;
		}
	}
	gsiToVec[irq] = vec;

	struct IoApicInfo *ioApic = findApic(irq);
	if (!ioApic) {
		printk("[IOAPIC] Could not find APIC for irq: %d", irq);
		return 1;
	}

	acquireSpinlock(&(ioApic->lock));
	uint32_t index = REG_IORED_BASE + ((irq - ioApic->gsiBase) * 2);
	*(ioApic->indexPort) = index;
	if ((flags & HWIRQ_FLAG_CUSTOM) == 0) {
		unsigned int prevFlags = *(ioApic->dataPort);
		flags = prevFlags & (HWIRQ_FLAG_POLARITY | HWIRQ_FLAG_TRIGGER);
	} else {
		flags &= HWIRQ_FLAG_POLARITY | HWIRQ_FLAG_TRIGGER;
	}
	uint64_t apicID = pcpuRead32(apicID);
	uint64_t value = flags | (vec & 0xFF) | (apicID << IORED_APIC_ID_SHIFT);

	*(ioApic->dataPort) = value;
	index++;
	*(ioApic->indexPort) = index;
	*(ioApic->dataPort) = value >> 32;
	releaseSpinlock(&(ioApic->lock));
	return 0;
}

void unrouteIrqLine(unsigned int irq, bool isa) {
	//mask ioapic entry
	if (isa) {
		for (unsigned int i = 0; i < isaListLen; i++) {
			if (isaOverrides[i].source == irq) {
				irq = isaOverrides[i].GSI;
				break;
			}
		}
	}
	struct IoApicInfo *ioApic = findApic(irq);
	if (!ioApic) {
		return;
	}
	acquireSpinlock(&(ioApic->lock));
	*(ioApic->indexPort) = REG_IORED_BASE + ((irq - ioApic->gsiBase) * 2);
	*(ioApic->dataPort) |= IORED_FLAG_MASK;
	releaseSpinlock(&(ioApic->lock));
}

int addISAOverride(uint32_t dst, uint16_t src, uint16_t flags) {
	if (isaListLen % ISA_LIST_INC == 0) {
		//alloc more room
		struct IsaOverride *oldOverrides = isaOverrides;
		isaOverrides = krealloc(isaOverrides, (isaListLen + ISA_LIST_INC) * sizeof(struct IsaOverride));
		if (isaOverrides == NULL) {
			isaOverrides = oldOverrides;
			return -1;
		}
	}
	isaOverrides[isaListLen].source = src;
	isaOverrides[isaListLen].flags = flags;
	isaOverrides[isaListLen].GSI = dst;
	isaListLen++;
	return 0;
}