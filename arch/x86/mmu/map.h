#ifndef MAP_H
#define MAP_H

#include <global.h>
#include <mm/paging.h>

typedef uint32_t PDE_t;
typedef uint32_t PTE_t;


/*
This function creates a new page table and initialises it to 0.
Note that the vaddr must be >= 0xFFC00000
*/
void newPageTable(virtPage_t vaddr);

/*
This function returns a pointer to a PDE with its matching vaddr
*/
PDE_t *getPDE(virtPage_t vaddr);

/*
This function returns a pointer to a PTE with its matching vaddr
*/
PTE_t *getPTE(virtPage_t vaddr);

#endif
