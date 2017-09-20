#ifndef ACPI_H
#define ACPI_H

#include <stdbool.h>
#include <stddef.h>

#define ACPI_LOG(text) printk("[ACPI] %s\n", text)

#define ACPI_WARN(text) printk("[ACPI] %s\n", text)

/*
Verifies the checksum on a acpi table
returns true if checksum is ok
*/
bool acpiVerifyChecksum(void *struc, size_t size);

#endif