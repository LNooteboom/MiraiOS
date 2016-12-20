#include "map.h"
#include <mm/pagemap.h>

#include <global.h>
#include <mm/paging.h>
#include <mm/physpaging.h>
#include <print.h>

#define EDIT_PML4T_BASE		0xFFFFFF7FBFDFE000
#define EDIT_PDPT_BASE		0xFFFFFF7FBFC00000
#define EDIT_PDT_BASE		0xFFFFFF7F80000000
#define EDIT_PT_BASE		0xFFFFFE0000000000

#define PAGE_BIT_WIDTH		9	//The number of bits a page level covers

#define PE_MASK			0xFFFFFFFFFFFFFFF8

#define PAGE_FLAG_PRESENT 	(1 << 0)
#define PAGE_FLAG_WRITE 	(1 << 1)
#define PAGE_FLAG_USER 		(1 << 2)

const pageTableLength = PAGESIZE / sizeof(pte_t);

static inline void setPDPTE(uintptr_t addr, pte_t entry) {
	addr = (addr >> (PAGE_BIT_WIDTH * 3) & PE_MASK | EDIT_PDPT_BASE;
	pte_t *entryPtr = (pte_t*)addr;
	*entryPtr = entry;
}
static inline physPage_t getPDPTE(uintptr_t addr) {
	addr = (addr >> (PAGE_BIT_WIDTH * 3) & PE_MASK | EDIT_PDPT_BASE;
	pte_t *entryPtr = (pte_t*)addr;
	return *entryPtr;
}

static inline void setPDE(uintptr_t addr, pte_t entry) {
	addr = (addr >> (PAGE_BIT_WIDTH * 2) & PE_MASK | EDIT_PDT_BASE;
	pte_t *entryPtr = (pte_t*)addr;
	*entryPtr = entry;
}
static inline physPage_t getPDE(uintptr_t addr) {
	addr = (addr >> (PAGE_BIT_WIDTH * 2) & PE_MASK | EDIT_PDT_BASE;
	pte_t *entryPtr = (pte_t*)addr;
	return *entryPtr;
}

static inline void setPTE(uintptr_t addr, pte_t entry) {
	addr = (addr >> (PAGE_BIT_WIDTH * 1) & PE_MASK | EDIT_PT_BASE;
	pte_t *entryPtr = (pte_t*)addr;
	*entryPtr = entry;
}
static inline physPage_t getPTE(uintptr_t addr) {
	addr = (addr >> (PAGE_BIT_WIDTH * 1) & PE_MASK | EDIT_PT_BASE;
	pte_t *entryPtr = (pte_t*)addr;
	return *entryPtr;
}
