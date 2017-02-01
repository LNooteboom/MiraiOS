#ifndef INCLUDE_PAGING_H
#define INCLUDE_PAGING_H

#include <global.h>

typedef uintptr_t physPage_t;

#define PAGE_FLAG_WRITE		(1 << 0)
#define PAGE_FLAG_USER		(1 << 1)

/*
This function allocates a page at a specified address (must be 4kb aligned)
*/
uint8_t allocPage(uintptr_t addr);

/*
This function deallocates a page
*/
uint8_t deallocPage(uintptr_t addr);

#endif
