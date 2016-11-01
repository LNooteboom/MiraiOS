#ifndef MM_INIT_H
#define MM_INIT_H

#include <global.h>
#include <param/mmap.h>

/*
This function pushes all the pages in *mmap on the internal page stack
*/
void pageInit(struct mmap *mmap, uint32_t mmapSize);

#endif
