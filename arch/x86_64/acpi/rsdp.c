#include "rsdp.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mm/paging.h>
#include <acpi.h>

#define RSDPTR_BOUNDARY 16

const char RSDP_SIG[8] = "RSD PTR ";

static uint16_t *EBDASeg = (uint16_t*)(0x40E + (uintptr_t)&VMEM_OFFSET);

static bool verifyChecksum(void *struc, size_t size) {
	char *cstruc = (char*)struc;
	char result = 0;
	for (uintptr_t i = 0; i < size; i++) {
		result += cstruc[i];
	}
	return !((bool)result);
}

static struct RSDP *findRsdp(void) {
	uint64_t *pSig = (uint64_t*)RSDP_SIG;
	uint64_t sig = *pSig;
	struct RSDP *ret;
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
			ACPI_WARN("ERROR: Could not find RSDP!");
			while (true) {
				asm("hlt");
			}
		}
	}
	ACPI_LOG("Found RSDP at: ");
	hexprintln64((uint64_t)searchBase);
	
	if (verifyChecksum(searchBase, sizeof(struct RSDP))) {
		ACPI_LOG("RSDP checksum valid\n");
	} else {
		ACPI_WARN("WARN: RSDP checksum invalid\n");
	}
	return (struct RSDP*)searchBase;
}

uintptr_t acpiFindXsdt(void) {
	struct RSDP *pRsdp = findRsdp();
	return 0;
}