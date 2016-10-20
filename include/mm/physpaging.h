#ifndef INCLUDE_PHYSPAGING_H
#define INCLUDE_PHYSPAGING_H

#define PAGESIZE 4096

/*
Allocates a 4kb page and returns its virtual address
*/
physPage_t allocPhysPage(void);

/*
Deallocates a 4kb page
*/
void deallocPhysPage(physPage_t page);

#endif
