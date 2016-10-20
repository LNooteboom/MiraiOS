#ifndef INCLUDE_BOOTINFO_H
#define INCLUDE_BOOTINFO_H

#include <global.h>
#include <mm/paging.h> //for physPage_t

struct mmap {
	physPage_t baseLow;
	physPage_t baseHigh;
	size_t lengthLow;
	size_t lengthHigh;
	uint32_t type;
};

#endif
