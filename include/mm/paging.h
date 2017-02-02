#ifndef INCLUDE_PAGING_H
#define INCLUDE_PAGING_H

#include <global.h>

typedef uintptr_t physPage_t;

#define PAGE_FLAG_WRITE		(1 << 0)
#define PAGE_FLAG_USER		(1 << 1)

/*
This function allocates a clean page at a specified address (must be 4kb aligned)
*/
bool allocCleanPage(uintptr_t addr, uint8_t flags);

/*
This function allocates a page at a specified address (must be 4kb aligned)
*/
bool allocPage(uintptr_t addr, uint8_t flags);

/*
This function deallocates a page
*/
bool deallocPage(uintptr_t addr);

#endif
