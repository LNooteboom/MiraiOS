#ifndef INCLUDE_PAGEMAP_H
#define INCLUDE_PAGEMAP_H

#include <mm/paging.h>

/*
Maps a page with physical address paddr to the virtual address vaddr.
*/
void mmMapPage(uintptr_t vaddr, physPage_t paddr, physPageFlags_t flags);

/*
Maps a large page with physical address paddr to the virtual address vaddr.
*/
void mmMapLargePage(uintptr_t vaddr, physPage_t paddr, physPageFlags_t flags);

/*
Unmaps a page.
*/
void mmUnmapPage(uintptr_t vaddr);

/*
Finds a page entry and returns it
*/
physPage_t mmGetPageEntry(uintptr_t vaddr);

/*
Reserves a physical page in memory
*/
void mmSetPageFlags(uintptr_t vaddr, physPageFlags_t flags);

#endif
