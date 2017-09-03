#ifndef INCLUDE_MM_INIT_H
#define INCLUDE_MM_INIT_H

#include <stdint.h>
#include <stddef.h>

/*
These 'variables' are linker symbols and have no value
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
void mmInitPaging(void);

void mmInitVirtPaging(void);

#endif
