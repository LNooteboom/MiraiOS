#include <mm/paging.h>
#include "physpaging.h"

#include <global.h>
#include <mm/pagemap.h>

#define PAGESTACKSTART 0xFF700000
#define PAGESTACKLIMIT 0xFFC00000

#define PAGEENTRIESINPAGEMASK (PAGESIZE / PTRSIZE) - 1
#define PAGESTACKBORDERMASK PAGESIZE - 1

physPage_t *curPageStack = (physPage_t*)PAGESTACKSTART;
uint32_t pageStackPtr = 0;
bool newPageStackAlloced = false;
bool newPageStackDealloced = false;


physPage_t allocPhysPage(void) {
	if (pageStackPtr == 0) {
		//no more free memory :(
		return NULL;
	}
	if ((pageStackPtr & (PAGEENTRIESINPAGEMASK)) == PAGEENTRIESINPAGEMASK && !newPageStackDealloced) {
		//stack space becomes allocated page
		newPageStackDealloced = true;
		virtPage_t vaddr = (pageStackPtr+1) * PTRSIZE + PAGESTACKSTART;
		return getPhysPage(vaddr);
	}
	newPageStackDealloced = false;
	return curPageStack[pageStackPtr--];
}

void deallocPhysPage(physPage_t page) {
	if ((pageStackPtr & (PAGEENTRIESINPAGEMASK)) == PAGEENTRIESINPAGEMASK && !newPageStackAlloced) {
		//deallocated page becomes more stack space
		newPageStackAlloced = true;
		virtPage_t vaddr = (pageStackPtr+1) * PTRSIZE + PAGESTACKSTART;
		mapPage(vaddr, page);
	} else {
		newPageStackAlloced = false;
		curPageStack[++pageStackPtr] = page;
	}
}

