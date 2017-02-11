#ifndef INCLUDE_PAGING_H
#define INCLUDE_PAGING_H

#include <global.h>

typedef uintptr_t physPage_t;
typedef uint64_t pageFlags_t;

#define PAGE_SIZE 4096
#define LARGEPAGE_SIZE		(PAGE_SIZE * 512)

#define PAGE_FLAG_WRITE		(1 << 1)	//page is writeable
#define PAGE_FLAG_USER		(1 << 2)	//page is accessable by userspace
#define PAGE_FLAG_INUSE		(1 << 9)	//page will be allocated upon access
#define PAGE_FLAG_CLEAN		(1 << 10)	//page to be allocated must be clean
#define PAGE_FLAG_EXEC		(1UL << 63)	//page is executable

#define PAGE_FLAG_PRESENT 	(1 << 0)
#define PAGE_FLAG_SIZE		(1 << 7)
#define PAGE_FLAG_ALLOCED	(1 << 11)

/*
Allocates a set of contiguous virtual pages in kernel memory
*/
void *allocKPages(uint16_t nrofPages, pageFlags_t flags);

/*
Allocates a set of contiguous virtual pages at a specified address
*/
void allocPageAt(uintptr_t addr, uint16_t nrofPages, pageFlags_t flags);

#endif
