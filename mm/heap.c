#include <mm/heap.h>

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <mm/init.h>
#include <mm/paging.h>
#include <mm/pagemap.h>
#include <mm/memset.h>
#include <sched/spinlock.h>
#include <print.h>

#define VHEAPLIMIT	0xFFFFFFFFE0000000
#define HEAP_ALIGN	16

#define AREA_SIZE	(~(HEAP_ALIGN - 1))
#define AREA_INUSE	(1 << 0)

typedef uint64_t memArea_t;

memArea_t *heapStart = (void*)(0xFFFFFFFFC0000000);
size_t heapSize = PAGE_SIZE - sizeof(memArea_t);

spinlock_t heapLock = 0;

void initHeap(void) {
	allocPageAt(heapStart, PAGE_SIZE, PAGE_FLAG_INUSE | PAGE_FLAG_WRITE);
	memArea_t *footer = (void*)(heapStart) + heapSize - sizeof(memArea_t);
	heapStart++;
	*heapStart = heapSize - sizeof(memArea_t);
	*footer = *heapStart;
}

static inline memArea_t *getFooterFromHeader(memArea_t *header) {
	return ( (void*)(header) + (*header & AREA_SIZE) - sizeof(memArea_t));
}
static inline memArea_t *getHeaderFromFooter(memArea_t *footer) {
	return ( (void*)(footer) - (*footer & AREA_SIZE) + sizeof(memArea_t));
}

static void *heapAlloc(size_t size, memArea_t *heap, size_t heapSize) {
	//basic first-fit mm
	size_t totalSize = size + (sizeof(memArea_t) * 2);

	memArea_t *newHeader = heap;
	bool foundMem = false;
	while ( (uintptr_t)(newHeader) < (uintptr_t)(heap) + heapSize){
		//hexprint64(*newHeader);
		//cprint('-');
		if ( !(*newHeader & AREA_INUSE) && *newHeader >= totalSize) {
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
		if (!(*header & AREA_INUSE)) {
			//already freed
			return;
		}

		*header &= ~AREA_INUSE;
		memArea_t *footer = getFooterFromHeader(header);

		memArea_t *nextHeader = footer + 1;
		if (nextHeader < (heap + heapSize) && !(*nextHeader & AREA_INUSE)) {
			//merge higher
			memArea_t *nextFooter = getFooterFromHeader(nextHeader);
			*header += *nextHeader;
			*nextFooter = *header;
			footer = nextFooter;
		}
		*footer &= ~AREA_INUSE;

		memArea_t *prevFooter = header - 1;
		memArea_t *prevHeader = getHeaderFromFooter(prevFooter);
		if (*prevFooter && !(*prevFooter & AREA_INUSE)) { //if previous area is free
			//merge lower
			*prevHeader += *header;
			*footer = *prevHeader;
		}
	}
}

void *kmalloc(size_t size) {
	//asm volatile ("xchg bx, bx");
	if (size == 0) {
		return NULL;
	}
	if (size % HEAP_ALIGN) {
		size -= size % HEAP_ALIGN;
		size += HEAP_ALIGN;
	}
	acquireSpinlock(&heapLock);
	void *retVal = heapAlloc(size, heapStart, heapSize);
	if (!retVal) {
		//allocate more heap space
		uintptr_t newPage = (uintptr_t)(heapStart) + heapSize;
		if (newPage >= VHEAPLIMIT) {
			releaseSpinlock(&heapLock);
			return NULL;
		}
		allocPageAt((void*)newPage, PAGE_SIZE, PAGE_FLAG_INUSE | PAGE_FLAG_WRITE);

		//merge it with the heap
		size_t newSize = heapSize + PAGE_SIZE;
		memArea_t *oldFooter = (memArea_t*)(newPage - (sizeof(memArea_t) * 2));
		memArea_t *newFooter = (memArea_t*)(newPage + PAGE_SIZE - (sizeof(memArea_t) * 2));
		memArea_t *header;
		if (*oldFooter & AREA_INUSE) {
			header = (memArea_t*)(newPage - sizeof(memArea_t));
			*header = PAGE_SIZE;
			*newFooter = *header;
		} else {
			//area is free, merge it
			header = getHeaderFromFooter(oldFooter);
			*header += PAGE_SIZE + sizeof(memArea_t);
			*newFooter = *header;
		}
		heapSize = newSize;
		//try again
		retVal = heapAlloc(size, heapStart, heapSize);
	}
	releaseSpinlock(&heapLock);
	return retVal;
}

void kfree(void *addr) {
	if (addr == NULL) {
		return;
	} else if (addr > (void*)(heapStart) && (uintptr_t)(addr) < ((uintptr_t)(heapStart) + heapSize)) {
		acquireSpinlock(&heapLock);
		heapFree(addr, heapStart, heapSize);
		releaseSpinlock(&heapLock);
	}
}

void *krealloc(void *addr, size_t newSize) {
	if (addr == NULL) {
		return kmalloc(newSize);
	}
	if (newSize == 0) {
		kfree(addr);
		return NULL;
	}
	if (newSize % HEAP_ALIGN) {
		newSize -= newSize % HEAP_ALIGN;
		newSize += HEAP_ALIGN;
	}
	acquireSpinlock(&heapLock);
	//get entry after current one
	memArea_t *curHeader = (memArea_t*)(addr) - 1;
	memArea_t *nextHeader = getFooterFromHeader(curHeader);
	nextHeader++;
	size_t oldSize = (*curHeader & AREA_SIZE) - (sizeof(memArea_t) * 2);
	if (newSize <= oldSize) {
		//shrink
		size_t diff = oldSize - newSize;
		if (diff < HEAP_ALIGN * 4) {
			releaseSpinlock(&heapLock);
			return addr;
		}
		*curHeader = newSize | AREA_INUSE;
		memArea_t *curFooter = getFooterFromHeader(curHeader);
		*curFooter = *curHeader;

		memArea_t *newHeader = curFooter + 1;
		*newHeader = diff;
		memArea_t *newFooter = getFooterFromHeader(newHeader);
		*newFooter = *newHeader;
		releaseSpinlock(&heapLock);
		return addr;
	}
	//expand
	size_t moreNeeded = newSize - oldSize;
	//get next header, check if its free
	//memArea_t *curFooter = getFooterFromHeader(curHeader);
	//memArea_t *nextHeader = curFooter + 1;
	if (*nextHeader & AREA_INUSE || (*nextHeader & AREA_SIZE) < moreNeeded) {
		//alloc new area, copy and free old area
		void *newAddr = kmalloc(newSize);
		memcpy(newAddr, addr, oldSize);
		kfree(addr);
		releaseSpinlock(&heapLock);
		return newAddr;
	}
	memArea_t *nextFooter = getFooterFromHeader(nextHeader);
	*curHeader = (newSize + sizeof(memArea_t) * 2) | AREA_INUSE;
	if (moreNeeded == (*nextHeader & AREA_SIZE)) {
		//no split needed
		*nextFooter = *curHeader;
		releaseSpinlock(&heapLock);
		return addr;
	}
	//split needed
	memArea_t *newFooter = getFooterFromHeader(curHeader);
	*newFooter = *curHeader;
	*nextFooter -= newSize;
	*(newFooter + 1) = *nextFooter;
	releaseSpinlock(&heapLock);
	return addr;
}