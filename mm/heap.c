#include <mm/heap.h>

#include <global.h>
#include <mm/init.h>

struct memArea {
	char[4] magic;
	size_t size;
	struct memArea *lower;
	struct memArea *higher;

	/*
	if memory area is free, there is a struct freeArea following this
	*/
};
struct freeArea {
	struct memArea *lower;
	struct memArea *higher;
};

struct memArea *freeAreaTree;
struct memArea *freeAreaSizeTree;
struct memArea *usedAreaTree = NULL;

void initHeap(void) {
	//heap start is bss end
	freeAreaTree = &BSS_END_ADDR;
	freeAreaSizeTree = freeAreaTree;

	freeAreaTree->magic = "FREE";
	freeAreaTree->size = HEAPSIZE;
	freeAreaTree->lower = NULL;
	freeAreaTree->higher = NULL;

	struct freeArea *sizePtrs = (struct freeArea*)(freeArea + 1);
	sizePtrs->lower = NULL;
	sizePtrs->higher = NULL;
}


struct memArea *searchInMemTree(struct memArea *top, uintptr_t addr) {
	if (addr == (uintptr_t)(top)) {
		return top;
	} else if (addr > (uintptr_t)(top)) {
		if (top->higher) {
			return searchInMemTree(top->higher, addr);
		} else {
			return NULL;
		}
	} else {
		if (top->lower) {
			return searchInMemTree(top->lower, addr);
		} else {
			return NULL;
		}
	}
}

void createUsedArea(void *addr, size_t size) {
	size += sizeof(memArea);
	size += 
}
