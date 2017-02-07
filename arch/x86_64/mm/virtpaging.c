#include <mm/virtpaging.h>

#include <global.h>
#include <mm/paging.h>
#include <mm/init.h> //for BSS_END_ADDR
#include <mm/pagemap.h>
#include <spinlock.h>
#include "map.h"

#define KERNEL_VMEM_END 0xFFFFFFFFC0000000

spinlock_t KVMemLock;

static bool checkMemRange(uintptr_t vaddr, uint16_t nrofPages) {
	bool ret = true;
	for (uint16_t i = 0; i < nrofPages; i++) {
		bool present = true;
		//hexprint64(vaddr);
		for (int8_t level = NROF_PAGE_LEVELS - 1; level >= 0; level--) {
			pte_t pte = *mmGetEntry(vaddr, level);
			if (!(pte & (PAGE_FLAG_PRESENT | PAGE_FLAG_INUSE))) {
				present = false;
				break;
			} else if (pte & PAGE_FLAG_SIZE) {
				break;
			}
		}
		//sprint(": ");
		//hexprintln(present);
		if (present) {
			ret = false;
			break;
		}
		vaddr += PAGE_SIZE;
	}
	//hexprintln(ret);
	return ret;
}

uintptr_t allocKPages(uint16_t nrofPages, physPageFlags_t flags) {
	uintptr_t vaddr = (uintptr_t)&BSS_END_ADDR & ~(LARGEPAGE_SIZE - 1);
	bool foundPages = false;
	uint16_t nrofKMemPages = (KERNEL_VMEM_END - (uintptr_t)&BSS_END_ADDR) / PAGE_SIZE;
	uint16_t nrofTests = nrofKMemPages - nrofPages;
	acquireSpinlock(&KVMemLock);
	for (uint16_t i = 0; i < nrofTests; i++) {
		if (checkMemRange(vaddr, nrofPages)) {
			for (uint16_t j = 0; j < nrofPages; j++) {
				mmReservePage(vaddr + (j * PAGE_SIZE), flags);
			}
			foundPages = true;
			break;
		}
		vaddr += PAGE_SIZE;
	}
	releaseSpinlock(&KVMemLock);
	if (foundPages) {
		return vaddr;
	} else {
		return NULL;
	}
}