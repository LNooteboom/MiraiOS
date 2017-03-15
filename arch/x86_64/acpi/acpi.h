#ifndef ACPI_H
#define ACPI_H

#include <stdbool.h>
#include <stddef.h>

#define ACPI_LOG(text) do {				\
		sprint("[\e[32macpi\e[0m] ");	\
		sprint(text);					\
	} while(0);

#define ACPI_WARN(text) do {			\
		sprint("[\e[34macpi\e[0m] ");	\
		sprint(text);					\
	} while(0);

/*
Verifies the checksum on a acpi table
returns true if checksum is ok
*/
bool acpiVerifyChecksum(void *struc, size_t size);

#endif