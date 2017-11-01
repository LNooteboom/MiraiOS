#include <arch/acpi.h>
#include "acpi.h"

#include <stdint.h>
#include <stdbool.h>
#include <mm/memset.h>
#include <mm/paging.h>
#include <panic.h>
#include "rsdp.h"
#include "madt.h"
#include "header.h"

bool isXsdt;
struct AcpiHeader *rsdtHeader;
uint64_t *xsdtContents = NULL; //if isXsdt == true
uint32_t *rsdtContents = NULL; //if isXsdt == false
uint32_t rsdtLength;

bool acpiVerifyChecksum(void *struc, size_t size) {
	char *cstruc = (char*)struc;
	char result = 0;
	for (uintptr_t i = 0; i < size; i++) {
		result += cstruc[i];
	}
	return !((bool)result);
}

static uint64_t getRsdtEntryPaddr(unsigned int index) {
	if (index >= rsdtLength) {
		return 0;
	}
	if (isXsdt) {
		return xsdtContents[index];
	} else {
		return rsdtContents[index];
	}
}

static void getRsdtEntry(unsigned int index, char *name, size_t *len, uint64_t *paddr) {
	uint64_t paddr2 = getRsdtEntryPaddr(index);
	*paddr = paddr2;
	if (!paddr2) {
		return;
	}
	struct AcpiHeader *header = ioremap(paddr2, sizeof(struct AcpiHeader));
	if (!header) {
		panic("[ACPI][ERROR] ioremap failed\n");
	}
	memcpy(name, header->sig, ACPI_SIG_LEN);
	*len = header->length;
	iounmap(header, sizeof(struct AcpiHeader));
}

void acpiInit(void) {
	acpiGetRsdt(&rsdtHeader, &isXsdt);
	if (!acpiVerifyChecksum(rsdtHeader, rsdtHeader->length)) {
		ACPI_WARN("RSDT checksum invalid!\n");
	}
	if (isXsdt) {
		xsdtContents = (uint64_t*)((uintptr_t)rsdtHeader + sizeof(struct AcpiHeader));
		rsdtLength = (rsdtHeader->length - sizeof(struct AcpiHeader)) / 8;
	} else {
		rsdtContents = (uint32_t*)((uintptr_t)rsdtHeader + sizeof(struct AcpiHeader));
		rsdtLength = (rsdtHeader->length - sizeof(struct AcpiHeader)) / 4;
	}
	char buf[ACPI_SIG_LEN + 1];
	buf[ACPI_SIG_LEN] = 0;
	bool foundMadt = false;
	for (unsigned int i = 0; i < rsdtLength; i++) {
		size_t entryLen;
		uint64_t paddr;
		getRsdtEntry(i, buf, &entryLen, &paddr);
		printk("[ACPI] Found table: %s\n", buf);
		if (!paddr) {
			continue; //skip NULL entry
		}
		if (!foundMadt && !memcmp(buf, madtSig, ACPI_SIG_LEN)) {
			acpiMadtInit(paddr, entryLen);
			foundMadt = true;
		}
	}
	if (!foundMadt) {
		panic("No MADT found!\n");
	}
}