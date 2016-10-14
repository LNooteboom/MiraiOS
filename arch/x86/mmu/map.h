#ifndef MAP_H
#define MAP_H

#include <global.h>

typedef uint32_t PDE_t;
typedef uint32_t PTE_t;


/*
This function creates a new page table and initialises it to 0.
Note that the vaddr must be >= 0xFFC00000
*/
void newPageTable(void *vaddr);

/*
This function returns a pointer to a PDE with its matching vaddr
*/
PDE_t *getPDE(void *vaddr);

/*
This function returns a pointer to a PTE with its matching vaddr
*/
PTE_t *getPTE(void *vaddr);

#endif
