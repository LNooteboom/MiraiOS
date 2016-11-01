#ifndef INCLUDE_PARAM_H
#define INCLUDE_PARAM_H

#include <global.h>

struct multiBootInfo {
	uint32_t flags;

	//if flags[0] is set
	size_t memLower;
	size_t memUpper;

	//if flags[1] is set
	uint8_t drive;
	uint8_t part1;
	uint8_t part2;
	uint8_t part3;

	//if flags[2] is set
	char *cmdLine;

	//if flags[3] is set
	uintptr_t modsAddr;
	uint32_t modsCount;

	//if flags[4] or flags[5] is set
	size_t tabSize;
	size_t strSize;
	void *symAddr;
	uint32_t reserved;

	//if flags[6] is set
	uint32_t mmapSize;
	struct mmap *mmap;

	//if flags[7] is set
	size_t drivesSize;
	void *drivesAddr;

	//if flags[8] is set
	void *configTable;

	//if flags[9] is set
	char *bootloaderName;

	//if flags[10] is set
	void *apmTable;

	//if flags[11] is set
	void *vbeControlInfo;
	void *vbeModeInfo;
	uint32_t vbeMode;
	uint32_t vbeInterfaceSeg;
	void *vbeInterfaceOff;
	size_t vbeInterfaceSize;
};

extern struct multiBootInfo *bootInfo;

#endif
