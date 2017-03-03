#ifndef RSDT_H
#define RSDT_H

#include <stdint.h>

struct RSDP {
	char sig[8];
	uint8_t checksum;
	char OEMID[6];
	uint8_t revision;
	uint32_t RsdtAddr;

	uint32_t tableLen;
	uint64_t xsdtAddr;
	uint8_t extChecksum;
	uint8_t reserved[3];
} __attribute__ ((packed));

uintptr_t acpiFindXsdt(void);

#endif