#ifndef HEADER_H
#define HEADER_H

#include <stdint.h>

struct acpiHeader {
	char sig[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char OEMID[6];
	char OEMTableID[8];
	uint32_t OEMRevision;
	uint32_t creatorID;
	uint32_t creatorRevision;
} __attribute__ ((packed));

#endif