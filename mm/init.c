#include <mm/init.h>

#include "heap.h"

#include <global.h>
#include <param/main.h>
#include <print.h>

void mmInit(void) {
	if (bootInfo->flags & (1 << 11)) {
		mmInitPaging((struct mmap*)(paramMmap), bootInfo->mmapSize);
	} else {
		sprint("\e[44mmmap is required but not present");
		while(true);
	}

	//initHeap();
}

