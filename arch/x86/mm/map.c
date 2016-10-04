#include "map.h"
#include <pagemap.h>

#include <paging.h>

#define PAGEDIRVADDR 0xFFFFF000

#define PDEINDEXMASK 0xFFC00000
//#define PDEINDEXSHIFT 22
#define PTEINDEXMASK 0x003FF000
#define PTEINDEXSHIFT 12

#define PDE_PTE_ADDRMASK 0xFFFFF000

#define PDE_PTE_PRESENT 1
#define PDE_PTE_WRITE 2
#define PDE_PTE_USER 4

#define PDEFLAGS PDE_PTE_PRESENT + PDE_PTE_WRITE + PDE_PTE_USER
#define PTEFLAGS PDEFLAGS

#define PAGEDIRSIZE (PAGESIZE / 4)

PDE_t *pageDir = PAGEDIRVADDR;

size_t pageTableSize = 0;

void setupPaging(void) {
}

void newPageTable(void *vaddr) {
	if (((uintptr_t)(vaddr) & PDEINDEXMASK) != PDEINDEXMASK) {
		panic("Attempted to create page table outside boundary.");
		return;
	}
	PDE_t newPT = (PDE_t*) allocPhysPage();
	newPT |= PDEFLAGS;
	//map the new PDE
	uint32_t index = (((uintptr_t)(vaddr) & PTEINDEXMASK) >> PTEINDEXSHIFT);
	pageDir[index] = newPT;

	//initialize everything to 0
	PTE_t *table = (PTE_t*) vaddr;
	for (uint32_t i = 0; i < PAGEDIRSIZE; i++) {
		table[i] = 0;
	}
}

void setInPageMaps(void *vaddr, void *paddr) {
	uint32_t indexPD = ((uintptr_t)(vaddr) & PDEINDEXMASK) >> PDEINDEXSHIFT;
	PDE_t entryPD = pageDir[indexPD];
	//check if a page table exists with that address
	if (entryPD & PDE_PTE_PRESENT == 0) {
		//page table does not exist, so create one
		newPageTable((void*)(entryPD & PDE_PTE_ADDRMASK));
	}
	PTE_t *entryPT = (uintptr_t)(vaddr) & PTEINDEXMASK | PDEINDEXMASK;
	*entryPT = (uintptr_t)(paddr) | PTEFLAGS;
}
