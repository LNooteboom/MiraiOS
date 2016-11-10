#include <mm/init.h>

#include "heap.h"

#include <global.h>
#include <param/mmap.h>
#include <param/bootinfo.h>
#include <print.h>

void initMm(void) {
	if (bootInfo->flags & (1 << 11)) {
		pageInit((struct mmap*)(bootInfo->mmap), bootInfo->mmapSize);
	} else {
		sprint("\e[44mmmap is required but not present");
		while(true);
	}

	initHeap();
}

