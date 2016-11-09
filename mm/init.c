#include "init.h"
#include <mm/init.h>

#include <global.h>
#include <param/mmap.h>
#include <param/bootinfo.h>
#include <mm/physpaging.h>
#include <print.h>

#define ENTRYTYPE_FREE 1

void initMm(void) {
	if (bootInfo->flags & (0 << 11)) {
		pageInit(bootInfo->mmap, bootInfo->mmapSize);
	} else {
		sprint("\e[44mmmap is required but not present");
		while(true);
	}
}

void pageInit(struct mmap *mmap, uint32_t mmapSize) {
	
	struct mmap *currentEntry = mmap;
	//for (uint32_t entryNr = 0; entryNr < mmapSize; entryNr++) {
	while ((uintptr_t)currentEntry < (uintptr_t)mmap + mmapSize) {
		/*if (currentEntry->type == ENTRYTYPE_FREE) {
			physPage_t currentPage = currentEntry->baseLow;
			for (size_t i = 0; i < currentEntry->lengthLow; i += PAGESIZE) {
				deallocPhysPage(currentPage);
				currentPage += PAGESIZE;
			}
		}*/
		sprint("Base: ");
		hexprintln(currentEntry->baseLow);
		sprint("Length: ");
		hexprintln(currentEntry->lengthLow);
		sprint("Type: ");
		hexprintln(currentEntry->type);
		cprint('\n');

		currentEntry = (struct mmap*)((void*)(currentEntry) + currentEntry->entrySize + sizeof(size_t));
	}
}
