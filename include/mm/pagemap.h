#ifndef INCLUDE_PAGEMAP_H
#define INCLUDE_PAGEMAP_H

#include <mm/paging.h>

/*
This function maps the physical page paddr to virtual address vaddr
*/
void mapPage(uintptr_t vaddr, physPage_t paddr);

/*
This function removes an entry from the page table
*/
void unmapPage(uintptr_t vaddr);

/*
This function returns looks up a virtual page address and returns a physical page address
*/
physPage_t getPhysPage(uintptr_t vaddr);

#endif
