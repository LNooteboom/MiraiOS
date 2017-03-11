#include "madt.h"

#include <stdint.h>
#include <stddef.h>
#include "acpi.h"
#include <mm/paging.h>
#include <print.h>
#include <pio.h>
#include <irq.h>

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
		ACPI_WARN("WARN: MADT checksum invalid!");
	}

	if (madtHdr->flags & 1) {
		//disable legacy PICs
		outb(0xa1, 0xff);
		outb(0x21, 0xff);
		ACPI_LOG("Legacy PICs disabled.");
	}

	madtContents = (char*)((uintptr_t)madtHdr + sizeof(struct madtHeader));
	size_t contentLen = madtLen - sizeof(struct madtHeader);
	unsigned int i = 0;
	while (i < contentLen) {
		struct recordHeader *recHeader = (struct recordHeader*)(&(madtContents[i]));
		if (recHeader->entryType == 0) {
			struct LAPIC *rec = (struct LAPIC*)recHeader;
			ACPI_LOG("Found local APIC ID: ");
			hexprintln(rec->apicID);
		} else if (recHeader->entryType == 1) {
			struct IOAPIC *rec = (struct IOAPIC*)recHeader;
			ACPI_LOG("Found IO APIC ID: ");
			hexprintln(rec->id);
		} else if (recHeader->entryType == 2) {
			struct irqSourceOvrr *rec = (struct irqSourceOvrr*)recHeader;
			if (rec->bus == IRQBUS_ISA) {
				addISAOverride(rec->GSI, rec->irqSource, rec->flags);
			}
		}
		i += recHeader->len;
	}
}