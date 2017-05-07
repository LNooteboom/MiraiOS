#include "madt.h"

#include <stdint.h>
#include <stddef.h>
#include "acpi.h"
#include <mm/paging.h>
#include <mm/heap.h>
#include <mm/memset.h>
#include <print.h>
#include <pio.h>
#include <irq.h>
#include <arch/cpu.h>
#include <arch/ioapic.h>

#define ioWait() outb(0x80, 0)

#define APIC_BUFFER_SIZE	256

struct madtHeader {
	struct acpiHeader acpi;
	uint32_t localCntAddr;
	uint32_t flags;
} __attribute__ ((packed));

struct recordHeader {
	uint8_t entryType;
	uint8_t len;
} __attribute__ ((packed));
struct LAPIC {
	struct recordHeader header;
	uint8_t acpiProcID;
	uint8_t apicID;
	uint32_t flags;
} __attribute__ ((packed));
struct IOAPIC {
	struct recordHeader header;
	uint8_t id;
	uint8_t reserved;
	uint32_t addr;
	uint32_t GSIIRQBase;
} __attribute__ ((packed));

#define IRQBUS_ISA	0
struct irqSourceOvrr {
	struct recordHeader header;
	uint8_t bus;
	uint8_t irqSource;
	uint32_t GSI;
	uint16_t flags;
} __attribute__ ((packed));

char madtSig[4] = "APIC";

struct madtHeader *madtHdr;
char *madtContents;

void acpiMadtInit(uint64_t madtPaddr, size_t madtLen) {
	madtHdr = ioremap(madtPaddr, madtLen);
	if (!acpiVerifyChecksum(madtHdr, madtLen)) {
		ACPI_WARN("WARN: MADT checksum invalid!\n");
	}

	if (madtHdr->flags & 1) {
		//disable pit
		outb(0x43, 0x30);
		outb(0x40, 0x00);
		outb(0x40, 0x00);

		//disable legacy PICs
		outb(0x20, 0x11);
		ioWait();
		outb(0xa0, 0x11);
		ioWait();
		outb(0x21, 16);
		ioWait();
		outb(0xa1, 24);
		ioWait();
		outb(0x21, 4);
		ioWait();
		outb(0xa1, 2);
		ioWait();
		outb(0x21, 0x01);
		ioWait();
		outb(0xa1, 0x01);
		ioWait();

		outb(0xa1, 0xff);
		outb(0x21, 0xff);
		//outb(0x20, 0x20);
		ACPI_LOG("Legacy PICs disabled.\n");
	}

	madtContents = (char*)((uintptr_t)madtHdr + sizeof(struct madtHeader));
	size_t contentLen = madtLen - sizeof(struct madtHeader);
	uint8_t apicIDs[APIC_BUFFER_SIZE];
	unsigned int i = 0;
	while (i < contentLen) {
		struct recordHeader *recHeader = (struct recordHeader*)(&(madtContents[i]));
		if (recHeader->entryType == 0) {
			struct LAPIC *rec = (struct LAPIC*)recHeader;
			ACPI_LOG("Found local APIC ID: ");
			hexprintln(rec->apicID);
			if (nrofCPUs < APIC_BUFFER_SIZE) {
				apicIDs[nrofCPUs++] = rec->apicID;
			}
		} else if (recHeader->entryType == 1) {
			struct IOAPIC *rec = (struct IOAPIC*)recHeader;
			ACPI_LOG("Found IO APIC ID: ");
			hexprintln(rec->id);
			struct ioApicInfo *oldInfos = ioApicInfos;
			ioApicInfos = krealloc(ioApicInfos, (nrofIOApics + 1) * sizeof(struct ioApicInfo));
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
			struct irqSourceOvrr *rec = (struct irqSourceOvrr*)recHeader;
			if (rec->bus == IRQBUS_ISA) {
				addISAOverride(rec->GSI, rec->irqSource, rec->flags);
			}
		}
		i += recHeader->len;
	}
	cpuInfos = kmalloc(nrofCPUs * sizeof(struct cpuInfo));
	if (!cpuInfos) {
		sprint("No memory available for cpuInfo!\n");
		while(1);
	}
	
	memset(cpuInfos, 0, nrofCPUs * sizeof(struct cpuInfo));
	for (unsigned int i = 0; i < nrofCPUs; i++) {
		cpuInfos[i].currentThread = NULL;
		cpuInfos[i].apicID = apicIDs[i];
		cpuInfos[i].cpuInfosIndex = i;
		cpuInfos[i].excStackTop = (void*)((uintptr_t)allocKPages(PAGE_SIZE, PAGE_FLAG_INUSE) + PAGE_SIZE);
		*((uint32_t*)(cpuInfos[i].excStackTop) - 4) = 0; //prealloc exception stack
	}
}