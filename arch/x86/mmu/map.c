#include "map.h"
#include <mm/pagemap.h>

#include <global.h>
#include <mm/paging.h>
#include <mm/physpaging.h>
#include <print.h>

#define PAGEDIRVADDR 0xFFFFF000

#define PDEINDEXMASK 0xFFC00000
#define PDEINDEXSHIFT 22
#define PTEINDEXMASK 0x003FF000
#define PTEINDEXSHIFT 12

#define PDE_PTE_ADDRMASK 0xFFFFF000

#define PDE_PTE_PRESENT 1
#define PDE_PTE_WRITE 2
#define PDE_PTE_USER 4

#define PDEFLAGS (PDE_PTE_PRESENT + PDE_PTE_WRITE + PDE_PTE_USER)
#define PTEFLAGS PDEFLAGS

#define PAGEDIRSIZE (PAGESIZE / 4)
#define PAGETABLESIZE PAGEDIRSIZE

PDE_t *pageDir = (PDE_t*)PAGEDIRVADDR;

void setInPageDir(virtPage_t index, physPage_t pt) {
	pt |= PDEFLAGS;
	pageDir[(index & PDEINDEXMASK) >> PDEINDEXSHIFT] = pt;
}

void newPageTable(virtPage_t vaddr) {
	vaddr &= PDE_PTE_ADDRMASK;
	physPage_t paddr = allocPhysPage();

	if (paddr == NULL) {
		sprint("Could not allocate physical page\n");
		return;
	}
	setInPageDir(vaddr, paddr);

	vaddr >>= (PDEINDEXSHIFT - PTEINDEXSHIFT);
	vaddr &= PTEINDEXMASK;
	vaddr |= PDEINDEXMASK;
	
	PTE_t *newPt = (PTE_t *)vaddr;
	for (uint16_t i = 0; i < PAGETABLESIZE; i++) {
		newPt[i] = 0;
	}
}

PDE_t *getPDE(virtPage_t vaddr) {
	uint32_t indexPD = (vaddr & PDEINDEXMASK) >> PDEINDEXSHIFT;
	return (pageDir + indexPD);
}
PTE_t *getPTE(virtPage_t vaddr) {
	vaddr &= PDE_PTE_ADDRMASK;
	return (PTE_t*)((vaddr >> (PDEINDEXSHIFT - PTEINDEXSHIFT)) | PDEINDEXMASK);
}

void mapPage(virtPage_t vaddr, physPage_t paddr) {
	PDE_t entryPD = *(getPDE(vaddr));
	//check if a page table exists with that address
	if ((entryPD & PDE_PTE_PRESENT) == 0) {
		//page table does not exist, so create one
		newPageTable(entryPD & PDE_PTE_ADDRMASK);
	}
	PTE_t *entryPT = getPTE(vaddr);
	*entryPT = paddr | PTEFLAGS;
}

void unmapPage(virtPage_t vaddr) {
	PTE_t *entryPT = getPTE(vaddr);
	*entryPT &= ~(PDE_PTE_PRESENT);
}

physPage_t getPhysPage(virtPage_t vaddr) {
	PTE_t entryPT = *(getPTE(vaddr));
	if (entryPT & PDE_PTE_PRESENT) {
		entryPT &= PDE_PTE_ADDRMASK;
		return entryPT;
	}
	return NULL;
}
