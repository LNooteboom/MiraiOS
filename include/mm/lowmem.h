#ifndef INCLUDE_LOWMEM_H
#define INCLUDE_LOWMEM_H

#include <stdint.h>
#include <mm/paging.h>

#define LOWMEM_SIZE			0x01000000

/*
Allocates a number of pages with a physical address < LOWMEM_SIZE.
*/
physPage_t allocLowPhysPages(uint8_t nrofPages);

/*
Frees a page of memory allocated by allocLowPhysPages
*/
void deallocLowPhysPages(physPage_t page, uint16_t nrofPages);

#endif