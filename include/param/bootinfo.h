#ifndef INCLUDE_PARAM_H
#define INCLUDE_PARAM_H

#include <global.h>

struct multiBootInfo {
	uint32_t flags;

	//if flags[0] is set
	uint32_t memLower;
	uint32_t memUpper;

	//if flags[1] is set
	uint8_t drive;
	uint8_t part1;
	uint8_t part2;
	uint8_t part3;

	//if flags[2] is set
	uint32_t cmdLine;

	//if flags[3] is set
	uint32_t modsAddr;
	uint32_t modsCount;

	//if flags[4] or flags[5] is set
	uint32_t tabSize;
	uint32_t strSize;
	uint32_t symAddr;
	uint32_t reserved;

	//if flags[6] is set
	uint32_t mmapSize;
	//struct mmap *mmap;
	uint32_t mmap;

	//if flags[7] is set
	uint32_t drivesSize;
	uint32_t drivesAddr;

	//if flags[8] is set
	uint32_t configTable;

	//if flags[9] is set
	uint32_t bootloaderName;

	//if flags[10] is set
	uint32_t apmTable;

	//if flags[11] is set
	uint32_t vbeControlInfo;
	uint32_t vbeModeInfo;
	uint32_t vbeMode;
	uint32_t vbeInterfaceSeg;
	uint32_t vbeInterfaceOff;
	uint32_t vbeInterfaceSize;
};

extern struct multiBootInfo *bootInfo;

#endif
