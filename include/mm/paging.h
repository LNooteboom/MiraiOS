#ifndef INCLUDE_PAGING_H
#define INCLUDE_PAGING_H

#include <global.h>

typedef uintptr_t physPage_t;
typedef uint16_t physPageFlags_t;

#define PAGE_SIZE 4096
#define LARGEPAGE_SIZE (PAGE_SIZE * 512)

#define PAGE_FLAG_WRITE		(1 << 1)
#define PAGE_FLAG_USER		(1 << 2)

#define PAGE_FLAG_PRESENT 	(1 << 0)
#define PAGE_FLAG_SIZE		(1 << 7)
/*avl bits*/
#define PAGE_FLAG_INUSE		(1 << 9)
#define PAGE_FLAG_CLEAN		(1 << 10)
#define PAGE_FLAG_ALLOCED	(1 << 11)

/*
This function allocates a clean page at a specified address (must be 4kb aligned)
*/
bool allocCleanPage(uintptr_t addr, physPageFlags_t flags);

/*
This function allocates a page at a specified address (must be 4kb aligned)
*/
bool allocPage(uintptr_t addr, physPageFlags_t flags);

/*
This function deallocates a page
*/
bool deallocPage(uintptr_t addr);

#endif
