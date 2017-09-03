#include "madt.h"

#include <stdint.h>
#include <stddef.h>
#include "acpi.h"
#include <mm/paging.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <print.h>
#include <io.h>
#include <irq.h>
#include <arch/cpu.h>
#include <arch/ioapic.h>
#include <panic.h>

#define ioWait() out8(0x80, 0)

#define APIC_BUFFER_SIZE	256

struct MadtHeader {
	struct AcpiHeader acpi;
	uint32_t localCntAddr;
	uint32_t flags;
} __attribute__ ((packed));

struct RecordHeader {
	uint8_t entryType;
	uint8_t len;
} __attribute__ ((packed));
struct LAPIC {
	struct RecordHeader header;
	uint8_t acpiProcID;
	uint8_t apicID;
	uint32_t flags;
} __attribute__ ((packed));
struct IOAPIC {
	struct RecordHeader header;
	uint8_t id;
	uint8_t reserved;
	uint32_t addr;
	uint32_t GSIIRQBase;
} __attribute__ ((packed));

#define IRQBUS_ISA	0
struct IrqSourceOvrr {
	struct RecordHeader header;
	uint8_t bus;
	uint8_t irqSource;
	uint32_t GSI;
	uint16_t flags;
} __attribute__ ((packed));

char madtSig[4] = "APIC";

struct MadtHeader *madtHdr;
char *madtContents;

void acpiMadtInit(uint64_t madtPaddr, size_t madtLen) {
	madtHdr = ioremap(madtPaddr, madtLen);
	if (!acpiVerifyChecksum(madtHdr, madtLen)) {
		ACPI_WARN("WARN: MADT checksum invalid!\n");
	}

	if (madtHdr->flags & 1) {
		//disable pit
		out8(0x43, 0x30);
		out8(0x40, 0x00);
		out8(0x40, 0x00);

		//disable legacy PICs
		out8(0x20, 0x11);
		ioWait();
		out8(0xa0, 0x11);
		ioWait();
		out8(0x21, 0xF0);
		ioWait();
		out8(0xa1, 24);
		ioWait();
		out8(0x21, 4);
		ioWait();
		out8(0xa1, 2);
		ioWait();
		out8(0x21, 0x01);
		ioWait();
		out8(0xa1, 0x01);
		ioWait();

		out8(0xa1, 0xff);
		out8(0x21, 0xff);
		//ACPI_LOG("Legacy PICs disabled.\n");
	}

	madtContents = (char*)((uintptr_t)madtHdr + sizeof(struct MadtHeader));
	size_t contentLen = madtLen - sizeof(struct MadtHeader);
	uint8_t apicIDs[APIC_BUFFER_SIZE];
	unsigned int i = 0;
	while (i < contentLen) {
		struct RecordHeader *recHeader = (struct RecordHeader*)(&(madtContents[i]));
		if (recHeader->entryType == 0) {
			struct LAPIC *rec = (struct LAPIC*)recHeader;
			//ACPI_LOG("Found local APIC ID: ");
			//decprintln(rec->apicID);
			if (nrofCPUs < APIC_BUFFER_SIZE) {
				apicIDs[nrofCPUs++] = rec->apicID;
			}
		} else if (recHeader->entryType == 1) {
			struct IOAPIC *rec = (struct IOAPIC*)recHeader;
			//ACPI_LOG("Found IO APIC ID: ");
			//decprintln(rec->id);
			struct IoApicInfo *oldInfos = ioApicInfos;
			ioApicInfos = krealloc(ioApicInfos, (nrofIOApics + 1) * sizeof(struct IoApicInfo));
			if (ioApicInfos) {
				ioApicInfos[nrofIOApics].id = rec->id;
				ioApicInfos[nrofIOApics].paddr = rec->addr;
				ioApicInfos[nrofIOApics].gsiBase = rec->GSIIRQBase;
				ioApicInfos[nrofIOApics].lock = 0;
				nrofIOApics++;
			} else {
				ACPI_LOG("No memory available for IO apic info\n");
				ioApicInfos = oldInfos;
			}
		} else if (recHeader->entryType == 2) {
			struct IrqSourceOvrr *rec = (struct IrqSourceOvrr*)recHeader;
			if (rec->bus == IRQBUS_ISA) {
				addISAOverride(rec->GSI, rec->irqSource, rec->flags);
			}
		}
		i += recHeader->len;
	}
	cpuInfos = kmalloc(nrofCPUs * sizeof(struct CpuInfo));
	if (!cpuInfos) {
		panic("No memory available for cpuInfos!\n");
	}
	
	memset(cpuInfos, 0, nrofCPUs * sizeof(struct CpuInfo));
	for (unsigned int i = 0; i < nrofCPUs; i++) {
		cpuInfos[i].addr = &cpuInfos[i];
		cpuInfos[i].currentThread = NULL;
		cpuInfos[i].apicID = apicIDs[i];
		cpuInfos[i].cpuInfosIndex = i;
		cpuInfos[i].excStackTop = (void*)((uintptr_t)allocKPages(PAGE_SIZE * 2, PAGE_FLAG_INUSE) + PAGE_SIZE * 2);
		*((uint32_t*)(cpuInfos[i].excStackTop) - 4) = 0; //prealloc exception stack

		for (int j = 0; j < NROF_QUEUE_PRIORITIES; j++) {
			cpuInfos[i].readyList[j].first = NULL;
			cpuInfos[i].readyList[j].last = NULL;
			cpuInfos[i].readyList[j].nrofThreads = 0;
		}
		cpuInfos[i].readyListLock = 0;
		cpuInfos[i].threadLoad = 0;
		cpuInfos[i].nrofReadyThreads = 0;
	}
}