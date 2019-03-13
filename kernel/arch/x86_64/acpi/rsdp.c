#include "rsdp.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mm/paging.h>
#include <arch/acpi.h>
#include <panic.h>
#include "header.h"
#include "acpi.h"
#include <arch/bootinfo.h>

#define RSDPTR_BOUNDARY 16

uintptr_t acpiRsdpPhysAddr;

static const char RSDP_SIG[8] = "RSD PTR ";

static uint16_t *EBDASeg = (uint16_t*)(0x40E + (uintptr_t)&VMEM_OFFSET);

static struct RSDP *findRsdp(void) {
	uint64_t *pSig = (uint64_t*)RSDP_SIG;
	uint64_t sig = *pSig; //encode signature to integer for fast comparison
	uint64_t *searchBase;
	bool found = false;
	if (*EBDASeg) {
		//search EBDA
		searchBase = (uint64_t*)(((uintptr_t)(*EBDASeg) << 4) + (uintptr_t)&VMEM_OFFSET);
		for (int i = 0; i < (1024 / RSDPTR_BOUNDARY); i++) {
			if (*searchBase == sig) {
				found = true;
				break;
			}
			searchBase += RSDPTR_BOUNDARY / sizeof(*searchBase);
		}
	}
	if (!found) {
		//search BDA
		searchBase = (uint64_t*)(0xE0000 + (uintptr_t)&VMEM_OFFSET);
		while ((uintptr_t)searchBase < (0x100000 + (uintptr_t)&VMEM_OFFSET)) {
			if (*searchBase == sig) {
				found = true;
				break;
			}
			searchBase += RSDPTR_BOUNDARY / sizeof(*searchBase);
		}
		if (!found) {
			panic("Could not find RSDP!\n");
		}
	}
	printk("[ACPI] Found RSDP at %x\n", (uint64_t)searchBase);
	
	if (acpiVerifyChecksum(searchBase, sizeof(struct RSDP))) {
		ACPI_LOG("RSDP checksum valid");
	} else {
		ACPI_WARN("WARN: RSDP checksum invalid!");
	}
	return (struct RSDP*)searchBase;
}

void acpiGetRsdt(struct AcpiHeader **rsdt, bool *isXsdt) {
	struct RSDP *rsdp;
	if (bootInfo.rsdp) {
		acpiRsdpPhysAddr = bootInfo.rsdp;
		rsdp = ioremap(bootInfo.rsdp, sizeof(struct RSDP));
		ACPI_LOG("Using EFI configuration table for RSDP");
	} else {
		rsdp = findRsdp();
		acpiRsdpPhysAddr = (uintptr_t)rsdp - (uintptr_t)&VMEM_OFFSET;
	}
	*isXsdt = (rsdp->revision >= 2);
	//*isXsdt = false;

	//get table size
	struct AcpiHeader *tempHeader = ioremap(rsdp->rsdtAddr, sizeof(struct AcpiHeader));
	size_t tableSize = tempHeader->length;
	iounmap(tempHeader, sizeof(struct AcpiHeader));

	if (*isXsdt) {
		*rsdt = ioremap(rsdp->xsdtAddr, tableSize);
	} else {
		*rsdt = ioremap(rsdp->rsdtAddr, tableSize);
	}
	if (bootInfo.rsdp) {
		iounmap(rsdp, sizeof(struct RSDP));
	}
}