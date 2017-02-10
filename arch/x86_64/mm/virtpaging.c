#include <mm/paging.h>

#include <global.h>
#include <mm/init.h> //for BSS_END_ADDR
#include <mm/pagemap.h>
#include <spinlock.h>
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
	for (uint16_t i = 0; i < nrofPages; i++) {
		bool present = true;
		for (int8_t level = NROF_PAGE_LEVELS - 1; level >= 0; level--) {
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
		return NULL;
	}
}

void *allocKPages(uint16_t nrofPages, physPageFlags_t flags) {
	uintptr_t vaddr = (uintptr_t)&BSS_END_ADDR & ~(LARGEPAGE_SIZE - 1);
	vaddr += LARGEPAGE_SIZE;
	bool foundPages = false;
	uintptr_t lastPage = KERNEL_VMEM_END - (nrofPages * PAGE_SIZE);
	flags |= PAGE_FLAG_ALLOCED;

	acquireSpinlock(&KVMemLock);
	while (vaddr <= lastPage) {
		uintptr_t newAddr = checkMemRange(vaddr, nrofPages);
		if (!newAddr) {
			for (uint16_t j = 0; j < nrofPages; j++) {
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

void allocPageAt(uintptr_t addr, uint16_t nrofPages, physPageFlags_t flags) {
	flags |= PAGE_FLAG_ALLOCED;
	for (uint16_t i = 0; i < nrofPages; i++) {
		mmSetPageFlags(addr + (i * PAGE_SIZE), flags);
	}
}