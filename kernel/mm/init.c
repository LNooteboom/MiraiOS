#include <mm/init.h>
#include "heap.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <sched/process.h>

void mmInit(void) {
	mmInitPaging();
	mmInitVirtPaging();
	initHeap();
}