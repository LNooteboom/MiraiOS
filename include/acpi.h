#ifndef INCLUDE_ACPI_H
#define INCLUDE_ACPI_H

#include <print.h>

#define ACPI_LOG(text) do {				\
		sprint("[\e[32macpi\e[0m] ");	\
		sprint(text);					\
	} while(0);

#define ACPI_WARN(text) do {				\
		sprint("[\e[34macpi\e[0m] ");	\
		sprint(text);					\
	} while(0);

void acpiInit(void);

#endif