#include <mm/init.h>

#include <stdint.h>
#include <stdbool.h>
#include <arch/bootinfo.h>
#include <arch/map.h>
#include <mm/paging.h>
#include <mm/pagemap.h>
#include <mm/lowmem.h>
#include <mm/physpaging.h>

#define ENTRYTYPE_FREE 1
#define NROF_PAGE_STACKS 4

void mmInitPaging(void) {
	struct MmapEntry *mmap = bootInfo.mmap;
	unsigned int len = bootInfo.mmapLen;
	bool first = true;
	
	for (unsigned int i = 0; i < len; i++) {
		int type = mmap[i].attr;
		if (type != ENTRYTYPE_FREE) {
			continue;
		}
		uintptr_t addr = mmap[i].addr;
		uint64_t nrofPages = mmap[i].nrofPages;
		if (addr < LOWMEM_SIZE) {
			uintptr_t end = addr + nrofPages * PAGE_SIZE;
			if (end <= LOWMEM_SIZE) {
				deallocLowPhysPages(addr, nrofPages);
				continue;
			}
			uint64_t lowPages = (LOWMEM_SIZE - addr) / PAGE_SIZE;
			deallocLowPhysPages(addr, lowPages);
			addr = LOWMEM_SIZE;
			nrofPages -= lowPages;
		}

		if (first && nrofPages >= NROF_PAGE_STACKS + 1) {
			uintptr_t bssEnd = (uintptr_t)&BSS_END_ADDR;
			if (bssEnd % LARGEPAGE_SIZE) {
				bssEnd -= bssEnd % LARGEPAGE_SIZE;
				bssEnd += LARGEPAGE_SIZE;
			}
			mmSetPageEntry(bssEnd, 1, addr | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
			addr += PAGE_SIZE;
			nrofPages--;

			uintptr_t zeroBuffer = bssEnd;
			bssEnd += PAGE_SIZE;
			for (int j = 0; j < NROF_PAGE_STACKS; j++) {
				mmMapPage(bssEnd + j * PAGE_SIZE, addr, PAGE_FLAG_WRITE);
				addr += PAGE_SIZE;
			}

			nrofPages -= NROF_PAGE_STACKS;
			mmInitPhysPaging(bssEnd, zeroBuffer, 0);
			first = false;
		}

		totalPhysPages += nrofPages;
		//freePhysPages += nrofPages;

		//dealloc highmem
		while (nrofPages) {
			if ((addr & (LARGEPAGE_SIZE - 1)) || nrofPages < (LARGEPAGE_SIZE / PAGE_SIZE)) {
				deallocPhysPage(addr);
				addr += PAGE_SIZE;
				nrofPages--;
			} else {
				deallocLargePhysPage(addr);
				addr += LARGEPAGE_SIZE;
				nrofPages -= LARGEPAGE_SIZE / PAGE_SIZE;
			}
		}
	}
}