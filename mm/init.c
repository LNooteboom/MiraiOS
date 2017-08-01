#include <mm/init.h>

#include "heap.h"

#include <stdint.h>
#include <stdbool.h>

#include <arch/bootinfo.h>

void mmInit(void) {
	mmInitPaging();
	initHeap();
}

