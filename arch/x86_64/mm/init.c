#include <mm/init.h>

#include <global.h>
#include <param/mmap.h>
#include <mm/physpaging.h>
#include <mm/pagemap.h>
#include <mm/heap.h>
#include <print.h>

#include "map.h"

#define LOWMEM_END 0x00100000
#define ENTRYTYPE_FREE 1

#define PAGE_SIZE 4096
#define LARGEPAGE_SIZE 0x200000

#define NROF_PAGE_STACKS 4

void mmInitPaging(struct mmap *mmap, size_t mmapSize) {
	uintptr_t physBssEnd = ((uintptr_t) &BSS_END_ADDR) - (uintptr_t)&VMEM_OFFSET;
	struct mmap *currentEntry = mmap;
	bool firstPage = true;

	while ((uintptr_t)currentEntry < (uintptr_t)mmap + mmapSize) {
		if (currentEntry->type == ENTRYTYPE_FREE && (currentEntry->base + currentEntry->length) > physBssEnd) {

			uint64_t base = currentEntry->base;
			uint64_t size = currentEntry->length;

			sprint("Found free memory: ");
			hexprint(base >> 32);
			hexprint(base);
			sprint(" - ");
			hexprint((base + size) >> 32);
			hexprintln(base + size);
			
			//Ignore static allocated memory
			if (base < physBssEnd) {
				size_t diff = physBssEnd - base;
				size -= diff;
				base = physBssEnd;
			}
			//align base
			if (base & (PAGE_SIZE - 1)) {
				base &= ~(PAGE_SIZE - 1);
				base += PAGE_SIZE;
			}

			//allocate room in virtual address space for page stacks
			if (firstPage && size >= (NROF_PAGE_STACKS + 1) * PAGE_SIZE) {
				uintptr_t start = (uintptr_t)&BSS_END_ADDR;
				if (start & (LARGEPAGE_SIZE - 1)) { //align it with large pages
					start &= ~(LARGEPAGE_SIZE - 1);
					start += LARGEPAGE_SIZE;
				}
				uintptr_t freeBufferLarge = start;
				start += LARGEPAGE_SIZE;
				uintptr_t freeBufferSmall = start;
				start += PAGE_SIZE;

				//set new page table
				size -= (NROF_PAGE_STACKS + 1) * PAGE_SIZE;
				mmSetPageEntry(start, 1, base | PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
				base += PAGE_SIZE;

				//for (uintptr_t i = 0; i < NROF_PAGE_STACKS * PAGE_SIZE; i += PAGE_SIZE) {
				//	mmSetPageEntry(i + start, 0, base | MMU_FLAG_PRESENT | MMU_FLAG_WRITE);
				//	base += PAGE_SIZE;
				//}

				mmInitPhysPaging(start, freeBufferSmall, freeBufferLarge);
				firstPage = false;
			}
			
			uintptr_t currentPage = base;
			for (uint64_t i = 0; i < size; i += PAGE_SIZE) {
				if ( !(currentPage & (LARGEPAGE_SIZE - 1)) && size >= PAGE_SIZE) {
					//There is a better way to do this, but right now this is the simplest solution
					deallocLargePhysPage(currentPage);
					currentPage += LARGEPAGE_SIZE;
					i += LARGEPAGE_SIZE - PAGE_SIZE;
				} else {
					deallocPhysPage(currentPage);
					currentPage += PAGE_SIZE;
				}
			}
			
		}

		currentEntry = (struct mmap*)((void*)(currentEntry) + currentEntry->entrySize + sizeof(uint32_t));
	}
}
