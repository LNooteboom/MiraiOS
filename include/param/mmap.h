#ifndef INCLUDE_PARAM_MMAP_H
#define INCLUDE_PARAM_MMAP_H

#include <global.h>
#include <mm/paging.h> //for physPage_t

struct mmap {
	uint32_t entrySize;
	/*
	physPage_t baseLow;
	physPage_t baseHigh;
	size_t lengthLow;
	size_t lengthHigh;
	*/
	uint64_t base;
	uint64_t length;
	uint32_t type;
};

#endif
