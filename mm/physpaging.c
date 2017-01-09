#include <mm/physpaging.h>

#include <global.h>
#include <mm/pagemap.h>
#include <mm/paging.h>
#include <spinlock.h>
#include <print.h>

#define PAGE_SIZE				4096
#define LARGE_PAGE_SIZE 		(PAGESIZE * 1024)

physPage_t *pageStackPtr = (physPage_t*)(PAGESTACKSTART);
bool newPageStackAlloced = false;
bool newPageStackDealloced = false;
spinlock_t physPageLock = 0;

physPage_t *largePageStackPtr = (physPage_t*)(LARGEPAGESTACKSTART - 1);
spinlock_t largePhysPageLock = 0;


/*
Divides a large page into smaller pages
returns true if successful
*/
static inline bool splitLargePage(void) {
	physPage_t largePage = allocLargePhysPage();
	if (!largePage) {
		return false;
	}
	for (uint16_t i = 0; i < (LARGEPAGESIZE / PAGESIZE); i++) {
		deallocPhysPage(largePage);
		largePage += PAGESIZE;
	}
	return true;
}

physPage_t allocPhysPage(void) {
	if (pageStackPtr == (physPage_t*)(PAGESTACKSTART)) {
		if (!splitLargePage()) {
			//no more free memory :(
			return NULL;
		}
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
		if ((uintptr_t)pageStackPtr <= PAGESTACKLIMIT) {
			//page stack full
			//this should never happen
			return;
		}
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

physPage_t allocLargePhysPage(void) {
	if (pageStackPtr == (physPage_t*)(PAGESTACKSTART)) {
		return NULL;
	}
	
	acquireSpinlock(&largePhysPageLock);
	physPage_t retVal = *largePageStackPtr++;
	releaseSpinlock(&largePhysPageLock);
	return retVal;
}

void deallocLargePhysPage(physPage_t page) {
	if ((uintptr_t)(largePageStackPtr) <= LARGEPAGESTACKLIMIT) {
		//large page stack full, this should never happen
		return;
	}
	acquireSpinlock(&largePhysPageLock);
	largePageStackPtr--;
	*largePageStackPtr = page;
	releaseSpinlock(&largePhysPageLock);
}
