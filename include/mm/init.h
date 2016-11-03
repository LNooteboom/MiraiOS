#ifndef INCLUDE_MM_INIT_H
#define INCLUDE_MM_INIT_H

#include <global.h>
#include <param/mmap.h>

/*
This function initialises the memory manager and must be called upon boot
*/
void initMm(void);

/*
This function pushes all the pages in *mmap on the internal page stack
*/
void pageInit(struct mmap *mmap, uint32_t mmapSize);

#endif
