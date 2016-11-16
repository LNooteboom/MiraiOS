#ifndef INCLUDE_PHYSPAGING_H
#define INCLUDE_PHYSPAGING_H

#include <mm/paging.h>

#define PAGESIZE 4096
#define LARGEPAGESIZE PAGESIZE * 1024

#define PAGESTACKSTART 0xFBC00000
#define PAGESTACKLIMIT 0xFFC00000

/*
Allocates a 4kb page and returns its virtual address
*/
physPage_t allocPhysPage(void);

/*
Deallocates a 4kb page
*/
void deallocPhysPage(physPage_t page);

#endif
