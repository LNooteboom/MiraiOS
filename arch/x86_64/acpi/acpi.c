#include <acpi.h>

#include <stdint.h>
#include <stdbool.h>
#include "rsdp.h"

bool isXsdt;
struct acpiHeader *rsdtHeader;

void acpiInit(void) {
	acpiGetRsdt(&rsdtHeader, &isXsdt);
}