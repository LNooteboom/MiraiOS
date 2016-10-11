#ifndef INCLUDE_PHYSPAGING_H
#define INCLUDE_PHYSPAGING_H

#define PAGESIZE 4096

/*
Allocates a 4kb page and returns its virtual address
*/
void *allocPhysPage(void);

/*
Deallocates a 4kb page
*/
void deallocPhysPage(void *page);

#endif
