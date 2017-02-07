#ifndef MM_VIRTPAGING_H
#define MM_VIRTPAGING_H

#include <global.h>
#include <mm/paging.h>

/*
Allocates a set of contiguous virtual pages
*/
uintptr_t allocKPages(uint16_t nrofPages, physPageFlags_t flags);

#endif