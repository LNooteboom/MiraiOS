#ifndef INCLUDE_PAGEMAP_H
#define INCLUDE_PAGEMAP_H

#include <mm/paging.h>

#define NROF_PAGE_LEVELS	4

#define PAGE_FLAG_WRITE		(1 << 0)
#define PAGE_FLAG_USER		(1 << 1)

/*
Finds the entry in the page table at a specified level and sets it to a specified value.
*/
void mmSetPageEntry(uintptr_t addr, uint8_t level, pte_t entry);

/*
Finds the entry in the page table at a specified level and sets it to a specfied value if there is no existing entry there.
*/
void mmSetPageEntryIfNotExists(uintptr_t addr, uint8_t level, pte_t entry);

/*
Finds the entry in the page table at a specified level and returns it.
*/
pte_t mmGetPageEntry(uintptr_t addr, uint8_t level);

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

#endif
