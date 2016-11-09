#ifndef INCLUDE_PAGING_H
#define INCLUDE_PAGING_H

#include <global.h>

typedef uint32_t physPage_t;
typedef uint32_t virtPage_t;

/*
This function allocates a page at a specified address (must be 4kb aligned)
*/
uint8_t allocPage(virtPage_t addr);

/*
This function deallocates a page
*/
uint8_t deallocPage(virtPage_t addr);

#endif
