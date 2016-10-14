#include "map.h"
#include <mm/pagemap.h>

#include <global.h>
#include <mm/paging.h>
#include <mm/physpaging.h>

#define PAGEDIRVADDR 0xFFFFF000

#define PDEINDEXMASK 0xFFC00000
#define PDEINDEXSHIFT 22
#define PTEINDEXMASK 0x003FF000
#define PTEINDEXSHIFT 12

#define PDE_PTE_ADDRMASK 0xFFFFF000

#define PDE_PTE_PRESENT 1
#define PDE_PTE_WRITE 2
#define PDE_PTE_USER 4

#define PDEFLAGS PDE_PTE_PRESENT + PDE_PTE_WRITE + PDE_PTE_USER
#define PTEFLAGS PDEFLAGS

#define PAGEDIRSIZE (PAGESIZE / 4)

PDE_t *pageDir = (PDE_t*)PAGEDIRVADDR;

size_t pageTableSize = 0;

void setupPaging(void) {
}

void newPageTable(void *vaddr) {
	if (((uintptr_t)(vaddr) & PDEINDEXMASK) != PDEINDEXMASK) {
		//panic("Attempted to create page table outside boundary.");
		return;
	}
	PDE_t newPT = (PDE_t)allocPhysPage();
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

PDE_t *getPDE(void *vaddr) {
	uint32_t indexPD = ((uintptr_t)(vaddr) & PDEINDEXMASK) >> PDEINDEXSHIFT;
	return (pageDir + indexPD);
}
PTE_t *getPTE(void *vaddr) {
	return (PTE_t*)(((uintptr_t)(vaddr) & PTEINDEXMASK) | PDEINDEXMASK);
}

void mapPage(void *vaddr, void *paddr) {
	PDE_t entryPD = *(getPDE(vaddr));
	//check if a page table exists with that address
	if ((entryPD & PDE_PTE_PRESENT) == 0) {
		//page table does not exist, so create one
		newPageTable((void*)(entryPD & PDE_PTE_ADDRMASK));
	}
	PTE_t *entryPT = getPTE(vaddr);
	*entryPT = ((uintptr_t)(paddr)) | (PTEFLAGS);
}

void unmapPage(void *vaddr) {
	PTE_t *entryPT = getPTE(vaddr);
	*entryPT &= ~(PDE_PTE_PRESENT);
}

void *getPhysPage(void *vaddr) {
	PTE_t entryPT = *(getPTE(vaddr));
	if (entryPT & PDE_PTE_PRESENT) {
		entryPT &= PDE_PTE_ADDRMASK;
		return (void*)(entryPT);
	}
	return NULL;
}
