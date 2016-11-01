#ifndef INCLUDE_PARAM_MMAP_H
#define INCLUDE_PARAM_MMAP_H

#include <global.h>
#include <mm/paging.h> //for physPage_t

struct mmap {
	size_t entrySize;
	physPage_t baseLow;
	physPage_t baseHigh;
	size_t lengthLow;
	size_t lengthHigh;
	uint32_t type;
};

#endif
