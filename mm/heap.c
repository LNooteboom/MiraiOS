#include <mm/heap.h>

#include <global.h>
#include <mm/init.h>
#include <spinlock.h>

#define FREEMEM_MAGIC 0x1337BEEF

typedef size_t memArea_t;

memArea_t *heapStart;

spinlock_t heapLock = 0;

void initHeap(void) {
	heapStart = ((void*)(&BSS_END_ADDR) + sizeof(memArea_t));

	*heapStart = HEAPSIZE;
	*(heapStart + HEAPSIZE - sizeof(memArea_t)) = HEAPSIZE;
}

static inline memArea_t *getFooterFromHeader(memArea_t *header) {
	return ((void*)(header) + *header - sizeof(memArea_t));
}

void *kmalloc(size_t size) {
	//basic first-fit mm
	if (size == 0) {
		return NULL;
	}
	if (size & 7) {
		size &= 7;
		size += 8;
	}
	size_t totalSize = size + (sizeof(memArea_t) * 2);

	acquireSpinlock(&heapLock);

	memArea_t *newHeader = heapStart;
	bool foundMem = false;
	while ( (uintptr_t)(newHeader) < (uintptr_t)(heapStart) + HEAPSIZE){
		if ( !(*newHeader & 1) && *newHeader >= totalSize) {
			foundMem = true;
			break;
		} else {
			newHeader = (void*)(newHeader) + (*newHeader & ~1);
		}
	}
	if (!foundMem) {
		releaseSpinlock(&heapLock);
		return NULL;
	}

	memArea_t oldSize = *newHeader;
	memArea_t *freeFooter = getFooterFromHeader(newHeader);
	*newHeader = totalSize;
	memArea_t *newFooter = getFooterFromHeader(newHeader);

	if (totalSize != oldSize) {
		//split and create new header for other area
		memArea_t *freeHeader = (void*)(newHeader) + *newHeader;
		*freeHeader = oldSize - totalSize;
		*freeFooter = *freeHeader;
	}

	*newHeader |= 1;
	*newFooter = *newHeader;

	releaseSpinlock(&heapLock);
	return (void*)(newHeader + 1);
}

void kfree(void *addr) {
	if (addr >= (void*)(heapStart) && addr < ( (void*)(heapStart) + HEAPSIZE)) {
		memArea_t *header = (addr - sizeof(memArea_t));
		if (!(*header & 1)) {
			//already freed
			return;
		}

		acquireSpinlock(&heapLock);

		*header &= ~1;
		memArea_t *footer;

		memArea_t *nextHeader = (void*)(header) + *header;
		if (nextHeader < (heapStart + HEAPSIZE) && !(*nextHeader & 1)) {
			//merge higher
			memArea_t *nextFooter = ( (void*)(nextHeader) + *nextHeader - sizeof(memArea_t));
			*header += *nextHeader;
			*nextFooter = *header;
			footer = nextFooter;
		} else {
			footer = ( (void*)(header) + *header - sizeof(memArea_t));
		}
		*footer &= ~1;

		memArea_t *prevFooter = header - 1;
		memArea_t *prevHeader = (void*)(prevFooter) - *prevFooter + sizeof(memArea_t);
		if (prevFooter > heapStart && !( (uintptr_t)(prevFooter) & 1) ) { //if previous area is free
			//merge lower
			*prevHeader += *header;
			*footer = *prevHeader;
		}

		releaseSpinlock(&heapLock);
	}
}
