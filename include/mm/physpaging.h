#ifndef INCLUDE_PHYSPAGING_H
#define INCLUDE_PHYSPAGING_H

#include <mm/paging.h>

#define PAGE_STACK_START 		0xFFFFFE8000000000

#define LARGE_PAGE_STACK_START	0xFFFFFF0000000000

/*
Allocates a 4kb page and returns its virtual address
*/
physPage_t allocPhysPage(void);

/*
Deallocates a 4kb page
*/
void deallocPhysPage(physPage_t page);

physPage_t allocLargePhysPage(void);

void deallocLargePhysPage(physPage_t page);

#endif
