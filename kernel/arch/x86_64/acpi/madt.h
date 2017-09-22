#ifndef MADT_H
#define MADT_H

#include <stdint.h>
#include <stddef.h>
#include "header.h"

extern char madtSig[ACPI_SIG_LEN];

void acpiMadtInit(uint64_t madtPaddr, size_t madtLen);

#endif