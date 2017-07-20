#ifndef INCLUDE_ARCH_BOOTINFO_H
#define INCLUDE_ARCH_BOOTINFO_H

#include <stdint.h>
#include <stdbool.h>
#include <mm/physpaging.h>

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

	//framebuffer info
	physPage_t fbAddr;
	size_t fbSize;
	uint32_t fbXRes;
	uint32_t fbYres;
	uint32_t fbPitch;

	bool fbIsRgb;
	uint32_t fbBpp;
	uint8_t fbRSize;
	uint8_t fbRShift;
	uint8_t fbGSize;
	uint8_t fbGShift;
	uint8_t fbBSize;
	uint8_t fbBShift;
	uint8_t fbResSize;
	uint8_t fbResShift;
};

extern struct bootInfo bootInfo;

#endif