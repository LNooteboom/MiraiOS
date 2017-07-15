#ifndef INCLUDE_ARCH_BOOTINFO_H
#define INCLUDE_ARCH_BOOTINFO_H

#include <stdint.h>

struct mmapEntry {
	uint64_t addr;
	uint64_t nrofPages;
	uint32_t attr;
	uint32_t reserved;
};

struct bootInfo {
	struct mmapEntry *mmap;
	uint64_t mmapLen;
	char *cmdLine;
	void *lowMemReservedEnd;
	void *initrd;
	size_t initrdLen;
};

extern struct bootInfo bootInfo;

#endif