#ifndef INCLUDEPAGING_H
#define PAGING_H

#include <global.h>

/*
This function allocates a page at a specified address (must be 4kb aligned)
*/
uint8_t allocPage(void *addr);

/*
This function deallocates a page
*/
uint8_t deallocPage(void *addr);

#endif
