#include <mm/heap.h>

#include <stdint.h>
#include <stdbool.h>
#include <mm/init.h>
#include <mm/paging.h>
#include <mm/pagemap.h>
#include <spinlock.h>
#include <print.h>

#define VHEAPLIMIT 0xFFFFFFFFE0000000
#define PAGESIZE 4096

typedef size_t memArea_t;

memArea_t *vHeapStart = (void*)(0xFFFFFFFFC0000000);
size_t vHeapSize = PAGESIZE - 8;

spinlock_t vHeapLock = 0;

void initHeap(void) {
	allocPageAt(vHeapStart, PAGE_SIZE, PAGE_FLAG_INUSE | PAGE_FLAG_WRITE);
	memArea_t *footer = (void*)(vHeapStart) + vHeapSize - sizeof(memArea_t);
	vHeapStart++;
	*vHeapStart = vHeapSize - sizeof(memArea_t);
	*footer = *vHeapStart;
}

static inline memArea_t *getFooterFromHeader(memArea_t *header) {
	return ( (void*)(header) + *header - sizeof(memArea_t));
}
static inline memArea_t *getHeaderFromFooter(memArea_t *footer) {
	return ( (void*)(footer) - *footer + sizeof(memArea_t));
}

static void *heapAlloc(size_t size, memArea_t *heap, size_t heapSize) {
	//basic first-fit mm
	size_t totalSize = size + (sizeof(memArea_t) * 2);

	memArea_t *newHeader = heap;
	bool foundMem = false;
	while ( (uintptr_t)(newHeader) < (uintptr_t)(heap) + heapSize){
		if ( !(*newHeader & 1) && *newHeader >= totalSize) {
			foundMem = true;
			break;
		} else if (*newHeader == 0) {
			break;
		} else {
			newHeader = (void*)(newHeader) + (*newHeader & ~1);
		}
	}
	if (!foundMem) {
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

	return (void*)(newHeader + 1);
}

static void heapFree(void *addr, memArea_t *heap, size_t heapSize) {
	if (addr >= (void*)(heap) && addr < ( (void*)(heap) + heapSize)) {
		memArea_t *header = (addr - sizeof(memArea_t));
		if (!(*header & 1)) {
			//already freed
			return;
		}

		*header &= ~1;
		memArea_t *footer;

		memArea_t *nextHeader = (void*)(header) + *header;
		if (nextHeader < (heap + heapSize) && !(*nextHeader & 1)) {
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
		memArea_t *prevHeader = getHeaderFromFooter(prevFooter);
		if (prevFooter > heap && !( (uintptr_t)(prevFooter) & 1) ) { //if previous area is free
			//merge lower
			*prevHeader += *header;
			*footer = *prevHeader;
		}
	}
}

void *vmalloc(size_t size) {
	if (size == 0) {
		return NULL;
	}
	if (size & 15) {
		size &= 15;
		size += 16;
	}
	
	acquireSpinlock(&vHeapLock);
	void *retVal = heapAlloc(size, vHeapStart, vHeapSize);
	if (!retVal) {
		//allocate more heap space
		uintptr_t newPage = (uintptr_t)(vHeapStart) + vHeapSize;
		if (newPage >= VHEAPLIMIT) {
			releaseSpinlock(&vHeapLock);
			return NULL;
		}
		allocPageAt((void*)newPage, PAGE_SIZE, PAGE_FLAG_INUSE | PAGE_FLAG_WRITE);

		//merge it with the heap
		size_t newSize = vHeapSize + PAGESIZE;
		memArea_t *oldFooter = (memArea_t*)(newPage - (sizeof(memArea_t) * 2));
		memArea_t *newFooter = (memArea_t*)(newPage + PAGESIZE - (sizeof(memArea_t) * 2));
		memArea_t *header;
		if (*oldFooter & 1) {
			header = (memArea_t*)(newPage - sizeof(memArea_t));
			*header = PAGESIZE;
			*newFooter = *header;
		} else {
			//area is free, merge it
			header = getHeaderFromFooter(oldFooter);
			*header += PAGESIZE + sizeof(memArea_t);
			*newFooter = *header;
		}
		vHeapSize = newSize;
		//try again
		retVal = heapAlloc(size, vHeapStart, vHeapSize);
	}
	releaseSpinlock(&vHeapLock);
	return retVal;
}

void kfree(void *addr) {
	if (addr == NULL) {
		return;
	} else if (addr > (void*)(vHeapStart) && (uintptr_t)(addr) < ((uintptr_t)(vHeapStart) + vHeapSize)) {
		acquireSpinlock(&vHeapLock);
		heapFree(addr, vHeapStart, vHeapSize);
		releaseSpinlock(&vHeapLock);
	}
}
