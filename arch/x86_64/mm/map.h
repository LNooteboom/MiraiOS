#ifndef MAP_H
#define MAP_H

#include <global.h>
#include <mm/paging.h>

typedef uint64_t pte_t;


/*
This function creates a new PDE.
index: On what part of the address space to map the new pt
pt: The physical location of the page table
*/
void setInPageDir(uintptr_t index, physPage_t pt);

/*
This function creates a new page table and initialises it to 0.
vaddr: the part of the address space you want to map the pt to
*/
void newPageTable(uintptr_t vaddr);

/*
This function returns a pointer to a PDE with its matching vaddr
*/
PDE_t *getPDE(uintptr_t vaddr);

/*
This function returns a pointer to a PTE with its matching vaddr
*/
PTE_t *getPTE(uintptr_t vaddr);

#endif
