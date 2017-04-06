#ifndef INCLUDE_PAGING_H
#define INCLUDE_PAGING_H

#include <stdint.h>
#include <stddef.h>

typedef uintptr_t physPage_t;
typedef uint64_t pageFlags_t;

#define PAGE_SIZE			4096
#define LARGEPAGE_SIZE		(PAGE_SIZE * 512)

//user settings
#define PAGE_FLAG_WRITE		(1 << 1)	//page is writeable
#define PAGE_FLAG_USER		(1 << 2)	//page is accessable by userspace
#define PAGE_FLAG_INUSE		(1 << 9)	//page will be allocated upon access
#define PAGE_FLAG_CLEAN		(1 << 10)	//page to be allocated must be clean
#define PAGE_FLAG_EXEC		(1UL << 63)	//page is executable

//internal settings
#define PAGE_FLAG_PRESENT 	(1 << 0)
#define PAGE_FLAG_SIZE		(1 << 7)
#define PAGE_FLAG_ALLOCED	(1 << 11)

extern char VMEM_OFFSET;

/*
Allocates a set of contiguous virtual pages in kernel memory
*/
void *allocKPages(size_t size, pageFlags_t flags);

/*
Allocates a set of contiguous virtual pages at a specified address
*/
void allocPageAt(void *addr, size_t size, pageFlags_t flags);

/*
Deallocates a set of pages
*/
void deallocPages(void *addr, size_t size);

/*
Maps mmio at address paddr to an allocated vaddr and returns it
*/
void *ioremap(uintptr_t paddr, size_t size);

/*
Unmaps mmio mapped by ioremap
*/
void iounmap(void *addr, size_t size);

#endif
