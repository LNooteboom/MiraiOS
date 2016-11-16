#include <mm/physpaging.h>

#include <global.h>
#include <mm/pagemap.h>
#include <mm/paging.h>
#include <spinlock.h>

#include <print.h>

#define PAGEENTRIESINPAGEMASK (PAGESIZE / 4) - 1
#define PAGESTACKBORDERMASK PAGESIZE - 1

physPage_t *curPageStack = (physPage_t*)PAGESTACKSTART;
uint32_t pageStackPtr = 0;
bool newPageStackAlloced = false;
bool newPageStackDealloced = false;

spinlock_t physPageLock = 0;


physPage_t allocPhysPage(void) {
	if (pageStackPtr == 0) {
		//no more free memory :(
		return NULL;
	}
	acquireSpinlock(&physPageLock);
	if ((pageStackPtr & (PAGEENTRIESINPAGEMASK)) == PAGEENTRIESINPAGEMASK && !newPageStackDealloced) {
		//stack space becomes allocated page
		newPageStackDealloced = true;
		virtPage_t vaddr = (pageStackPtr+1) * sizeof(physPage_t) + PAGESTACKSTART;

		releaseSpinlock(&physPageLock);
		return getPhysPage(vaddr);
	}
	newPageStackDealloced = false;

	physPage_t retVal = curPageStack[pageStackPtr];
	pageStackPtr--;
	releaseSpinlock(&physPageLock);
	return retVal;
}

void deallocPhysPage(physPage_t page) {
	acquireSpinlock(&physPageLock);
	if ((pageStackPtr & (PAGEENTRIESINPAGEMASK)) == PAGEENTRIESINPAGEMASK && !newPageStackAlloced) {
		//deallocated page becomes more stack space
		newPageStackAlloced = true;
		virtPage_t vaddr = (pageStackPtr+1) * sizeof(physPage_t) + PAGESTACKSTART;
		mapPage(vaddr, page);
	} else {
		newPageStackAlloced = false;
		curPageStack[++pageStackPtr] = page;
	}
	releaseSpinlock(&physPageLock);
}
