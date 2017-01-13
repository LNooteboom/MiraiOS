#ifndef INCLUDE_PHYSPAGING_H
#define INCLUDE_PHYSPAGING_H

#include <global.h>
#include <mm/paging.h>

/*
Allocates a 4kb page and returns its virtual address
*/
physPage_t allocPhysPage(void);

/*
Deallocates a 4kb page
*/
void deallocPhysPage(physPage_t page);

physPage_t allocCleanPhysPage(void);

physPage_t allocLargePhysPage(void);

void deallocLargePhysPage(physPage_t page);

void mmInitPhysPaging(uintptr_t firstStack);

#endif
