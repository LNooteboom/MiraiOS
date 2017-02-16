#ifndef INCLUDE_MM_INIT_H
#define INCLUDE_MM_INIT_H

#include <global.h>
#include <param/mmap.h>

/*
This 'variable' is a linker symbol and has no value, it will give a page fault upon read/write.
*/
extern char BSS_END_ADDR;

extern char KERNEL_START_ADDR;


/*
This function initialises the memory manager and must be called upon boot
*/
void mmInit(void);

/*
This function pushes all the pages in *mmap on the internal page stack
*/
void mmInitPaging(struct mmap *mmap, size_t mmapSize);

#endif
