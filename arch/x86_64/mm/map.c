#include "map.h"
#include <mm/pagemap.h>

#include <global.h>
#include <mm/paging.h>
#include <mm/physpaging.h>
#include <print.h>

#define PAGE_BIT_WIDTH		9	//The number of bits a page level covers
#define PE_MASK			0xFFFFFFFFFFFFFFF8

#define MMU_FLAG_PRESENT 	(1 << 0)
#define MMU_FLAG_WRITE 		(1 << 1)
#define MMU_FLAG_USER 		(1 << 2)
#define MMU_FLAG_SIZE		(1 << 7)

static const pageTableLength = PAGESIZE / sizeof(pte_t);

static const uintptr_t pageLevelBase[NROF_PAGE_LEVELS] = {
	0xFFFFFE0000000000	//PT
	0xFFFFFF7F80000000	//PDT
	0xFFFFFF7FBFC00000	//PDPT
	0xFFFFFF7FBFDFE000	//PML4T
}

/*
Finds the entry in the page table at a specified level and sets it to a specified value.
*/
void mmSetPageEntry(uintptr_t addr, uint8_t level, pte_t entry) {
	addr = (addr >> (PAGE_BIT_WIDTH * level) & PE_MASK | pageLevelBase[level];
	pte_t *entryPtr = (pte_t*)addr;
	*entryPtr = entry;
}

/*
Finds the entry in the page table at a specified level and sets it to a specfied value if there is no existing entry there.
*/
uint8_t mmSetPageEntryIfNotExists(uintptr_t addr, uint8_t level, pte_t entry) {
	addr = (addr >> (PAGE_BIT_WIDTH * level) & PE_MASK | pageLevelBase[level];
	pte_t *entryPtr = (pte_t*)addr;
	if (!(*entryPtr & MMU_FLAG_PRESENT)) {
		*entryPtr = entry;
	}
}
/*
Finds the entry in the page table at a specified level and returns it.
*/
pte_t *mmGetPageEntry(uintptr_t addr, uint8_t level) {
	addr = (addr >> (PAGE_BIT_WIDTH * level) & PE_MASK | pageLevelBase[level];
	pte_t *entryPtr = (pte_t*)addr;
	return entryPtr;
}

/*
Maps a page with physical address paddr to the virtual address vaddr.
*/
void mmMapPage(uintptr_t vaddr, physPage_t paddr, uint8_t flags) {
	for (int8_t i = NROF_PAGE_LEVELS - 1; i >= 1; i--) {
		pte_t *entry = mmGetPageEntry(vaddr, i);
		if ( !(*entry & PAGE_FLAG_PRESENT)) {
			//Page entry higher does not exist
			physPage_t page = mmAllocZeroedPhysPage();
			*entry = page | MMU_FLAG_PRESENT | (flags << 1);
		}
	}
	pte_t entry = paddr | (flags << 1) | MMU_FLAG_PRESENT;
	mmSetPageEntry(vaddr, 0, entry);
	return;
}
/*
Maps a large page with physical address paddr to the virtual address vaddr.
*/
void mmMapLargePage(uintptr_t vaddr, physPage_t paddr, uint8_t flags) {
	for (int8_t i = NROF_PAGE_LEVELS - 1; i >= 2; i--) {
		pte_t entry = *mmGetPageEntry(vaddr, i);
		if ( !(entry & PAGE_FLAG_PRESENT)) {
			//Page entry higher does not exist
			physPage_t page = mmAllocZeroedPhysPage();
			*entry = page | MMU_FLAG_PRESENT | (flags << 1);
		}
	}
	pte_t entry = paddr | (flags << 1) + MMU_FLAG_PRESENT + MMU_FLAG_SIZE;
	mmSetPageEntry(vaddr, 1, entry);
	return;
}

/*
Unmaps a page.
*/
void mmUnmapPage(uintptr_t vaddr) {
	for (int8_t i = NROF_PAGE_LEVELS - 1; i >= 0; i--) {
		pte_t *entry = mmGetPageEntry(vaddr, i);
		if ( !(*entry & MMU_FLAG_PRESENT)) {
			//Page entry higher does not exist
			return;
		} else if (i == 0 || *entry & MMU_FLAG_SIZE) {
			*entry = 0;
			return;
		}
	}
}
