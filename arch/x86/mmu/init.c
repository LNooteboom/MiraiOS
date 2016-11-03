#include <mm/init.h>

#include <global.h>
#include <param/mmap.h>
#include <mm/physpaging.h>
#include <mm/pagemap.h>
#include <print.h>

#include "map.h"

#define LOWMEM_END 0x00100000
#define ENTRYTYPE_FREE 1

/*
This 'variable' is a linker symbol and has no value, it will give a page fault upon read/write.
*/
extern char BSS_END_ADDR;

void pageInit(struct mmap *mmap, uint32_t mmapSize) {
	uintptr_t bssEnd = ((uintptr_t) &BSS_END_ADDR) - 0xC0000000;
	bool firstPage = true;
	struct mmap *currentEntry = mmap;
	//for (uint32_t entryNr = 0; entryNr < mmapSize; entryNr++) {
	while ((uintptr_t)currentEntry < (uintptr_t)mmap + mmapSize) {
		if (currentEntry->type == ENTRYTYPE_FREE && (currentEntry->base + currentEntry->length) > bssEnd) {

			physPage_t currentPage = currentEntry->base;
			uint64_t size = currentEntry->length;

			if (currentPage < bssEnd) {
				size_t diff = bssEnd - currentPage;
				size -= diff;
				currentPage = bssEnd;
			}
			sprint("Entry: ");
			hexprintln(currentPage);
			sprint("Size: ");
			hexprintln(size);

			if (firstPage && size >= 2*PAGESIZE) {
				//alocate new page table
				setInPageDir(PAGESTACKSTART, currentPage);
				currentPage += PAGESIZE;

				//map page
				mapPage(PAGESTACKSTART, currentPage);
				currentPage += PAGESIZE;
				size -= 2*PAGESIZE;

				firstPage = false;
			}

			for (uint64_t i = 0; i < size; i += PAGESIZE) {
				deallocPhysPage(currentPage);
				currentPage += PAGESIZE;
			}
		}
		/*
		sprint("Base: ");
		hexprintln(currentEntry->base);
		sprint("Length: ");
		hexprintln(currentEntry->length);
		sprint("Type: ");
		hexprintln(currentEntry->type);
		cprint('\n');
		*/

		currentEntry = (struct mmap*)((void*)(currentEntry) + currentEntry->entrySize + sizeof(uint32_t));
	}
	sprint("Alloced: ");
	hexprintln(allocPhysPage());
}
