#include <mm/physpaging.h>

#include <global.h>
#include <mm/pagemap.h>
#include <mm/paging.h>
#include <spinlock.h>

#include <print.h>

#define PAGEENTRIESINPAGEMASK 0x0ffc

physPage_t *pageStackPtr = (physPage_t*)(PAGESTACKSTART) - 1;
bool newPageStackAlloced = false;
bool newPageStackDealloced = false;

spinlock_t physPageLock = 0;


physPage_t allocPhysPage(void) {
	if (pageStackPtr == (physPage_t*)(PAGESTACKSTART) - 1) {
		//no more free memory :(
		return NULL;
	}
	acquireSpinlock(&physPageLock);
	if ( !((uintptr_t)pageStackPtr & PAGEENTRIESINPAGEMASK) && !newPageStackDealloced) {
		//stack space becomes allocated page
		newPageStackDealloced = true;
		virtPage_t vaddr = (virtPage_t)(pageStackPtr) - PAGESIZE;

		releaseSpinlock(&physPageLock);
		return getPhysPage(vaddr);
	}
	newPageStackDealloced = false;

	physPage_t retVal = *pageStackPtr++;
	releaseSpinlock(&physPageLock);
	return retVal;
}

void deallocPhysPage(physPage_t page) {
	acquireSpinlock(&physPageLock);
	if ( !( (uintptr_t)pageStackPtr & PAGEENTRIESINPAGEMASK) && !newPageStackAlloced) {
		//deallocated page becomes more stack space
		newPageStackAlloced = true;
		virtPage_t vaddr = (virtPage_t)pageStackPtr - PAGESIZE;
		mapPage(vaddr, page);
	} else {
		newPageStackAlloced = false;
		pageStackPtr--;
		*pageStackPtr = page;
	}
	releaseSpinlock(&physPageLock);
}
