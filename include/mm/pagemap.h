#ifndef INCLUDE_PAGEMAP_H
#define INCLUDE_PAGEMAP_H

#include <mm/paging.h>

#define NROF_PAGE_LEVELS	4

#define PAGE_FLAG_WRITE		(1 << 0)
#define PAGE_FLAG_USER		(1 << 1)

/*
Maps a page with physical address paddr to the virtual address vaddr.
*/
void mmMapPage(uintptr_t vaddr, physPage_t paddr, uint8_t flags);

/*
Maps a large page with physical address paddr to the virtual address vaddr.
*/
void mmMapLargePage(uintptr_t vaddr, physPage_t paddr, uint8_t flags);

/*
Unmaps a page.
*/
void mmUnmapPage(uintptr_t vaddr);

/*
Finds a page entry and returns it
*/
physPage_t mmGetPageEntry(uintptr_t vaddr);

#endif
