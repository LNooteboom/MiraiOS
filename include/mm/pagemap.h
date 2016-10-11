#ifndef INCLUDE_PAGEMAP_H
#define INCLUDE_PAGEMAP_H

/*
This function maps the physical page paddr to virtual address vaddr
*/
void mapPage(void *vaddr, void *paddr);

/*
This function removes an entry from the page table
*/
void unmapPage(void *vaddr);

/*
This function returns looks up a virtual page address and returns a physical page address
*/
void *getPhysPage(void *vaddr);

#endif
