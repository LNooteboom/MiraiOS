#ifndef INCLUDE_ARCH_ACPI_H
#define INCLUDE_ARCH_ACPI_H

#include <stdint.h>

/*
Reads the ACPI tables and calls the correct drivers
*/
void acpiInit(void);

extern uintptr_t acpiRsdpPhysAddr;

#endif