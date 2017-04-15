#include <mm/paging.h>

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <mm/init.h> //for BSS_END_ADDR
#include <mm/pagemap.h>
#include <sched/spinlock.h>
#include <mm/physpaging.h>
#include "map.h"

#define KERNEL_VMEM_END 0xFFFFFFFFC0000000

spinlock_t KVMemLock;

/*
Checks a memory range starting at vaddr
returns NULL if available, address of present page if not available
*/
static uintptr_t checkMemRange(uintptr_t vaddr, uint16_t nrofPages) {
	bool ret = true;
	bool large = false;
	for (int i = 0; i < nrofPages; i++) {
		bool present = true;
		for (int level = NROF_PAGE_LEVELS - 1; level >= 0; level--) {
			pte_t pte = *mmGetEntry(vaddr, level);
			if (!(pte & (PAGE_FLAG_PRESENT | PAGE_FLAG_INUSE | PAGE_FLAG_ALLOCED))) {
				present = false;
				break;
			} else if (pte & PAGE_FLAG_SIZE) {
				large = true;
				break;
			}
		}
		if (present) {
			ret = false;
			break;
		}
		vaddr += PAGE_SIZE;
	}
	if (!ret) {
		if (large) {
			vaddr += LARGEPAGE_SIZE;
		} else {
			vaddr += PAGE_SIZE;
		}
		return vaddr;
	} else {
		return 0;
	}
}

void *allocKPages(size_t size, pageFlags_t flags) {
	unsigned int nrofPages = size / PAGE_SIZE;
	if (size % PAGE_SIZE) {
		nrofPages++;
	}
	uintptr_t vaddr = (uintptr_t)&BSS_END_ADDR & ~(LARGEPAGE_SIZE - 1);
	vaddr += LARGEPAGE_SIZE;
	bool foundPages = false;
	uintptr_t lastPage = KERNEL_VMEM_END - (nrofPages * PAGE_SIZE);
	flags |= PAGE_FLAG_ALLOCED;

	acquireSpinlock(&KVMemLock);
	while (vaddr <= lastPage) {
		uintptr_t newAddr = checkMemRange(vaddr, nrofPages);
		if (!newAddr) {
			for (unsigned int j = 0; j < nrofPages; j++) {
				mmSetPageFlags(vaddr + (j * PAGE_SIZE), flags);
			}
			foundPages = true;
			break;
		}
		vaddr = newAddr;
	}
	releaseSpinlock(&KVMemLock);

	if (foundPages) {
		return (void*)vaddr;
	} else {
		return NULL;
	}
}

void allocPageAt(void *addr, size_t size, pageFlags_t flags) {
	unsigned int nrofPages = size / PAGE_SIZE;
	if (size % PAGE_SIZE) {
		nrofPages++;
	}
	flags |= PAGE_FLAG_ALLOCED;
	for (unsigned int i = 0; i < nrofPages; i++) {
		mmSetPageFlags((uintptr_t)addr + (i * PAGE_SIZE), flags);
	}
}

void deallocPages(void *addr, size_t size) {
	unsigned int nrofPages = size / PAGE_SIZE;
	if (size % PAGE_SIZE) {
		nrofPages++;
	}
	for (unsigned int i = 0; i < nrofPages; i++) {
		pte_t *entry = mmGetEntry((uintptr_t)addr + (i * PAGE_SIZE), 0);
		if ((*entry & PAGE_FLAG_PRESENT) && (*entry & PAGE_FLAG_INUSE) && (*entry & PAGE_MASK)) {
			physPage_t page = *entry & PAGE_MASK;
			deallocPhysPage(page);
		}
		*entry = 0;
	}
}

void *ioremap(uintptr_t paddr, size_t size) {
	unsigned int paddrDiff = paddr % PAGE_SIZE;
	size += paddrDiff;
	unsigned int nrofPages = size / PAGE_SIZE;
	if (size % PAGE_SIZE) {
		nrofPages++;
	}
	uintptr_t ret = (uintptr_t)allocKPages(size, PAGE_FLAG_WRITE);
	ret += paddrDiff;
	for (unsigned int i = 0; i < nrofPages; i++) {
		mmMapPage(ret + (i * PAGE_SIZE), paddr - paddrDiff, PAGE_FLAG_WRITE);
		paddr += PAGE_SIZE;
	}
	return (void*)ret;
}
void iounmap(void *addr, size_t size) {
	unsigned int addrDiff = (uintptr_t)addr % PAGE_SIZE;
	size += addrDiff;
	addr = (void*)((uintptr_t)addr - addrDiff);
	unsigned int nrofPages = size / PAGE_SIZE;
	if (size % PAGE_SIZE) {
		nrofPages++;
	}
	for (unsigned int i = 0; i < nrofPages; i++) {
		mmUnmapPage((uintptr_t)addr + (i * PAGE_SIZE));
	}
}