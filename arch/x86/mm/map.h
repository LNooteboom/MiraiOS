#ifdef MAP_H
#define MAP_H

typedef uint32_t PDE_t;
typedef uint32_t PTE_t;


/*
This function creates a new page table and initialises it to 0.
Note that the vaddr must be >= 0xFFC00000
*/
void newPageTable(void *vaddr);

#endif
