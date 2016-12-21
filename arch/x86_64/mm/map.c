#include "map.h"
#include <mm/pagemap.h>

#include <global.h>
#include <mm/paging.h>
#include <mm/physpaging.h>
#include <print.h>

#define PAGE_BIT_WIDTH		9	//The number of bits a page level covers

#define PE_MASK			0xFFFFFFFFFFFFFFF8

#define NROF_PAGE_LEVELS

#define MMU_FLAG_PRESENT 	(1 << 0)
#define MMU_FLAG_WRITE 		(1 << 1)
#define MMU_FLAG_USER 		(1 << 2)
#define MMU_FLAG_SIZE		(1 << 7)

#define PAGE_FLAG_WRITE		(1 << 0)
#define PAGE_FLAG_USER		(1 << 1)

static const pageTableLength = PAGESIZE / sizeof(pte_t);

static const uintptr_t pageLevelBase[NROF_PAGE_LEVELS] = {
	0xFFFFFE0000000000	//PT
	0xFFFFFF7F80000000	//PDT
	0xFFFFFF7FBFC00000	//PDPT
	0xFFFFFF7FBFDFE000	//PML4T
}

/*
This function finds the entry in the page table at a specified level and sets it to a specified value
The right address which uses the recursive slot in the PML4T is calculated first, and then the value at that
address is returned.
----
addr: The address to get the page table entry for
level: The level of the page table
entry: The entry to be set in the page table
*/
void mmSetPageEntry(uintptr_t addr, uint8_t level, pte_t entry) {
	addr = (addr >> (PAGE_BIT_WIDTH * level) & PE_MASK | pageLevelBase[level];
	pte_t *entryPtr = (pte_t*)addr;
	*entryPtr = entry;
}
/*
This function finds the entry in the page table at a specified level and returns it.
The right address which uses the recursive slot in the PML4T is calculated first, and then the value at that
address is returned.
----
addr: The address to get the page table entry for
level: The level of the page table
----
returns the entry in the page table
*/
pte_t *mmGetPageEntry(uintptr_t addr, uint8_t level) {
	addr = (addr >> (PAGE_BIT_WIDTH * level) & PE_MASK | pageLevelBase[level];
	pte_t *entryPtr = (pte_t*)addr;
	return entryPtr;
}

/*
This function maps a page with physical address paddr to the virtual address vaddr.
Whether the higher page levels exists is checked first, if they don't exist, they get automatically allocated and initialized.
Then the paddr and the flags are set in the lowest page table at the entry specified by vaddr
----
vaddr: The virtual page to map the pysical address to
paddr: The pysical page to be mapped
flags: The flags for the mmu which specify whether the page is read-only etc.
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
This function maps a large page with physical address paddr to the virtual address vaddr.
Whether the higher page levels exists is checked first, if they don't exist, they get automatically allocated and initialized.
Then the paddr and the flags are set in the second lowest page table at the entry specified by vaddr
----
vaddr: The virtual page to map the pysical address to
paddr: The pysical page to be mapped
flags: The flags for the mmu which specify whether the page is read-only etc.
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
This function unmaps a page.
The lowest page table entry that exists is cleared to 0
----
vaddr: The page to be unmapped
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
