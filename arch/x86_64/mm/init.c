#include <mm/init.h>

#include <global.h>
#include <param/mmap.h>
#include <mm/physpaging.h>
#include <mm/pagemap.h>
#include <mm/heap.h>
#include <print.h>

//#include "map.h"

#define LOWMEM_END 0x00100000
#define ENTRYTYPE_FREE 1

void pageInit(struct mmap *mmap, size_t mmapSize) {
	uintptr_t bssEnd = ((uintptr_t) &BSS_END_ADDR) - (uintptr_t)&VMEM_OFFSET + HEAPSIZE;
	bool firstPage = true;
	struct mmap *currentEntry = mmap;
	while ((uintptr_t)currentEntry < (uintptr_t)mmap + mmapSize) {
		if (currentEntry->type == ENTRYTYPE_FREE && (currentEntry->base + currentEntry->length) > bssEnd) {

			uint64_t base = currentEntry->base;
			uint64_t size = currentEntry->length;

			sprint("Found free memory: ");
			hexprint(base >> 32);
			hexprint(base);
			sprint(" - ");
			hexprint((base + size) >> 32);
			hexprintln(base + size);
			
			//Ignore static allocated memory
			if (base < bssEnd) {
				size_t diff = bssEnd - base;
				size -= diff;
				base = bssEnd;
			}

			//insert
			physPage_t currentPage = base + size - PAGE_SIZE;

			if (firstPage && size >= NROF_PAGE_LEVELS * 2 * PAGE_SIZE) {
				//map page stack
				for (int8_t i = NROF_PAGE_LEVELS - 1; i >= 0; i++) {
					
					pte_t entry = base + PAGE_FLAG_WRITE;
					mmSetPageEntryIfNotExists(PAGE_STACK_START, i, entry);
					base += PAGE_SIZE;
					size -= PAGE_SIZE;
				}
				
				//map large page stack
				for (int8_t i = NROF_PAGE_LEVELS - 1; i >= 0; i++) {
					pte_t entry = base + PAGE_FLAG_WRITE;
					mmSetPageEntryIfNotExists(LARGE_PAGE_STACK_START - PAGE_SIZE, i, entry);
					base += PAGE_SIZE;
					size -= PAGE_SIZE;
				}

				firstPage = false;
			}
			/*
			for (uint64_t i = 0; i < size; i += PAGE_SIZE) {
				if ( !(currentPage & (LARGEPAGE_SIZE - 1)) && size >= PAGE_SIZE) {
					//There is a better way to do this, but right now this is the simplest solution
					deallocLargePhysPage(currentPage);
					currentPage += LARGEPAGE_SIZE;
					i += LARGEPAGE_SIZE - PAGE_SIZE;
				} else {
					deallocPhysPage(currentPage);
					currentPage -= PAGE_SIZE;
				}
			}
			*/
		}

		currentEntry = (struct mmap*)((void*)(currentEntry) + currentEntry->entrySize + sizeof(uint32_t));
	}
}
