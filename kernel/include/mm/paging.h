#ifndef INCLUDE_PAGING_H
#define INCLUDE_PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdatomic.h>

typedef uintptr_t physPage_t;
typedef uint64_t pageFlags_t;

#define PAGE_SIZE			4096
#define LARGEPAGE_SIZE		(PAGE_SIZE * 512)

//permission settings
#define PAGE_FLAG_WRITE		(1 << 1)	//page is writeable
#define PAGE_FLAG_USER		(1 << 2)	//page is accessable by userspace
#define PAGE_FLAG_EXEC		(1UL << 63)	//page is executable
#define PAGE_FLAG_KERNEL	(1 << 8)	//global bit: disables tlb flush for kernel pages

//alloc settings
#define PAGE_FLAG_INUSE		(1 << 9)	//page will be allocated upon access
#define PAGE_FLAG_CLEAN		(1 << 10)	//page to be allocated must be clean

//shared page settings
#define PAGE_FLAG_SHARED	(1 << 11)
#define PAGE_FLAG_COW		(1 << 9)

//internal settings
#define PAGE_FLAG_PRESENT 	(1 << 0)
#define PAGE_FLAG_SIZE		(1 << 7)


extern char VMEM_OFFSET;

extern unsigned long totalPhysPages;
extern atomic_ulong freePhysPages;

static inline long sizeToPages(size_t size) {
	long ret = size / PAGE_SIZE;
	if (size % PAGE_SIZE) {
		ret++;
	}
	return ret;
}

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
