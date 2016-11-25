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

void pageInit(struct mmap *mmap, uint32_t mmapSize) {
	uintptr_t bssEnd = ((uintptr_t) &BSS_END_ADDR) - 0xC0000000 + HEAPSIZE;
	bool firstPage = true;
	struct mmap *currentEntry = mmap;
	//for (uint32_t entryNr = 0; entryNr < mmapSize; entryNr++) {
	while ((uintptr_t)currentEntry < (uintptr_t)mmap + mmapSize) {
		if (currentEntry->type == ENTRYTYPE_FREE && (currentEntry->base + currentEntry->length) > bssEnd) {

			physPage_t base = currentEntry->base;
			uint64_t size = currentEntry->length;

			sprint("Found free memory: ");
			hexprint(base);
			sprint(" - ");
			hexprintln(base + size);

			if (base < bssEnd) {
				size_t diff = bssEnd - base;
				size -= diff;
				base = bssEnd;
			}

			//insert
			physPage_t currentPage = base + size - PAGESIZE;

			if (firstPage && size >= 2*PAGESIZE) {
				//alocate new page table
				setInPageDir(PAGESTACKSTART - PAGESIZE, base);
				base += PAGESIZE;

				//map page
				mapPage(PAGESTACKSTART - PAGESIZE, base);
				base += PAGESIZE;
				
				//map large page stack
				mapPage(LARGEPAGESTACKSTART - PAGESIZE, base);
				size -= 2 * PAGESIZE;

				firstPage = false;
			}

			for (uint64_t i = 0; i < size; i += PAGESIZE) {
				if ( !(currentPage & (LARGEPAGESIZE - 1)) && size >= PAGESIZE) {
					//There is a better way to do this, but right now this is the simplest solution
					deallocLargePhysPage(currentPage);
					currentPage += LARGEPAGESIZE;
					i += LARGEPAGESIZE - PAGESIZE;
				} else {
					deallocPhysPage(currentPage);
					currentPage -= PAGESIZE;
				}
			}
		}

		currentEntry = (struct mmap*)((void*)(currentEntry) + currentEntry->entrySize + sizeof(uint32_t));
	}
}
