#include <mm/init.h>

#include "heap.h"

#include <stdint.h>
#include <stdbool.h>

void mmInit(void) {
	mmInitPaging();
	initHeap();
}

