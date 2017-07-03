#include <arch/map.h>
#include <mm/pagemap.h>

#include <stdint.h>
#include <stddef.h>
#include <mm/paging.h>
#include <mm/physpaging.h>
#include <print.h>

#define PAGE_BIT_WIDTH		9	//The number of bits a page level covers
#define PE_MASK			0xFFFFFFFFFFFFFFF8

static const uintptr_t pageLevelBase[NROF_PAGE_LEVELS] = {
	0xFFFFFF0000000000,	//PT
	0xFFFFFF7F80000000,	//PDT
	0xFFFFFF7FBFC00000,	//PDPT
	0xFFFFFF7FBFDFE000	//PML4T
};

/*
Finds the entry in the page table at a specified level and sets it to a specified value.
*/
void mmSetPageEntry(uintptr_t addr, uint8_t level, pte_t entry) {
	addr &= 0x0000FFFFFFFFFFFF;
	addr = (addr >> (PAGE_BIT_WIDTH * (level + 1)) & PE_MASK) | pageLevelBase[level];
	pte_t *entryPtr = (pte_t*)addr;
	*entryPtr = entry;
}

/*
Finds the entry in the page table at a specified level and returns it.
*/
pte_t *mmGetEntry(uintptr_t addr, uint8_t level) {
	addr &= 0x0000FFFFFFFFFFFF;
	addr = (addr >> (PAGE_BIT_WIDTH * (level + 1)) & PE_MASK) | pageLevelBase[level];
	pte_t *entryPtr = (pte_t*)addr;
	return entryPtr;
}

/*
Finds a page entry and returns it
*/
physPage_t mmGetPageEntry(uintptr_t vaddr) {
	for (int8_t i = NROF_PAGE_LEVELS - 1; i >= 0; i--) {
		pte_t *entry = mmGetEntry(vaddr, i);
		if ( !(*entry & PAGE_FLAG_PRESENT)) {
			//Page entry higher does not exist
			return 0;
		} else if (i == 0 || *entry & PAGE_FLAG_SIZE) {
			return (physPage_t)(*entry & PAGE_MASK);
		}
	}
	return 0;
}


/*
Maps a page with physical address paddr to the virtual address vaddr.
*/
void mmMapPage(uintptr_t vaddr, physPage_t paddr, pageFlags_t flags) {
	if (nxEnabled) {
		flags ^= PAGE_FLAG_EXEC; //flip exec bit to get NX bit
	} else {
		flags &= ~PAGE_FLAG_EXEC;
	}
	for (int8_t i = NROF_PAGE_LEVELS - 1; i >= 1; i--) {
		pte_t *entry = mmGetEntry(vaddr, i);
		if ( !(*entry & PAGE_FLAG_PRESENT)) {
			//Page entry higher does not exist
			physPage_t page = allocCleanPhysPage();
			*entry = page | PAGE_FLAG_PRESENT | flags;
		}
	}
	pte_t entry = paddr | flags | PAGE_FLAG_PRESENT | PAGE_FLAG_INUSE;
	mmSetPageEntry(vaddr, 0, entry);
	return;
}

/*
Maps a large page with physical address paddr to the virtual address vaddr.
*/
void mmMapLargePage(uintptr_t vaddr, physPage_t paddr, pageFlags_t flags) {
	if (nxEnabled) {
		flags ^= PAGE_FLAG_EXEC; //flip exec bit to get NX bit
	} else {
		flags &= ~PAGE_FLAG_EXEC;
	}
	for (int8_t i = NROF_PAGE_LEVELS - 1; i >= 2; i--) {
		pte_t *entry = mmGetEntry(vaddr, i);
		if ( !(*entry & PAGE_FLAG_PRESENT)) {
			//Page entry higher does not exist
			physPage_t page = allocCleanPhysPage();
			*entry = page | PAGE_FLAG_PRESENT | flags;
		}
	}
	pte_t entry = paddr | flags | PAGE_FLAG_PRESENT | PAGE_FLAG_INUSE | PAGE_FLAG_SIZE;
	mmSetPageEntry(vaddr, 1, entry);
	return;
}

void mmSetPageFlags(uintptr_t vaddr, pageFlags_t flags) {
	if (nxEnabled) {
		flags ^= PAGE_FLAG_EXEC; //flip exec bit to get NX bit
	} else {
		flags &= ~PAGE_FLAG_EXEC;
	}
	for (int8_t i = NROF_PAGE_LEVELS - 1; i >= 1; i--) {
		pte_t *entry = mmGetEntry(vaddr, i);
		if ( !(*entry & PAGE_FLAG_PRESENT)) {
			//Page entry higher does not exist
			physPage_t page = allocCleanPhysPage();
			*entry = page | PAGE_FLAG_PRESENT | flags;
		}
	}
	*mmGetEntry(vaddr, 0) = flags;
}

/*
Unmaps a page.
*/
void mmUnmapPage(uintptr_t vaddr) {
	mmSetPageFlags(vaddr, 0);
}

void mmUnmapBootPages(void) {
	*mmGetEntry(0, 2) = 0;
	*mmGetEntry(0, 3) = 0;
}