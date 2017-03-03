#ifndef INCLUDE_PHYSPAGING_H
#define INCLUDE_PHYSPAGING_H

#include <stdint.h>
#include <mm/paging.h>

#define PAGE_SIZE				4096
#define LARGE_PAGE_SIZE 		(PAGE_SIZE * 512)

/*
Initialises the page stacks
*/
void mmInitPhysPaging(uintptr_t firstStack, uintptr_t freeMemBufferSmall, uintptr_t freeMemBufferLarge);

/*
Allocates a small page and returns its physical address
*/
physPage_t allocPhysPage(void);

/*
Allocates a small zeroed page and returns its physical address
*/
physPage_t allocCleanPhysPage(void);

/*
Allocates a large page and returns its physical address
*/
physPage_t allocLargePhysPage(void);

/*
Allocates a zeroed large page and returns its physical address
*/
physPage_t allocLargeCleanPhysPage(void);

/*
Deallocates a small page
*/
void deallocPhysPage(physPage_t page);

/*
Deallocates a large page
*/
void deallocLargePhysPage(physPage_t page);

#endif
