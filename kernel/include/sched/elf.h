#ifndef INCLUDE_SCHED_ELF_H
#define INCLUDE_SCHED_ELF_H

#include <stdint.h>

struct ElfHeader {
	char magic[4];
	uint8_t bits;
	uint8_t endianness;
	uint8_t abiVer;
	uint8_t osABI;
	uint64_t unused;

	uint16_t type;
	uint16_t machine;
	uint32_t elfVer;
	uint64_t entryAddr;
	uint64_t phOff;
	uint64_t shOff;
	uint32_t flags;
	uint16_t ehSize;

	uint16_t phentSize;
	uint16_t phnum;
	uint16_t shentSize;
	uint16_t shnum;

	uint16_t shstrndx;
};

struct ElfPHEntry {
	uint32_t type;
	uint32_t flags;
	uint64_t pOffset;
	uint64_t pVAddr;
	uint64_t undefined;
	uint64_t pFileSz;
	uint64_t pMemSz;
	uint64_t alignment;
};

#endif