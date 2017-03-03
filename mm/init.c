#include <mm/init.h>

#include "heap.h"

#include <stdint.h>
#include <stdbool.h>
#include <param/main.h>
#include <print.h>

void mmInit(void) {
	if (bootInfo->flags & (1 << 6)) {
		mmInitPaging((struct mmap*)(paramMmap), bootInfo->mmapSize);
	} else {
		sprint("\e[44mmmap is required but not present\n\e[0m");
		while(true);
	}

	initHeap();
}

