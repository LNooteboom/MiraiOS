#include <mm/paging.h>
#include "physpaging.h"

#include <global.h>

#define PAGESTACKSTART 0xFF700000
#define PAGESTACKLIMIT 0xFFC00000

#define PAGEENTRIESINPAGEMASK (PAGESIZE / PTRSIZE) - 1
#define PAGESTACKBORDERMASK PAGESIZE - 1

void **curPageStack = (void**)PAGESTACKSTART;
uint32_t pageStackPtr = 0;
bool newPageStackAlloced = false;
bool newPageStackDealloced = false;


void *allocPhysPage(void) {
	if (pageStackPtr == 0) {
		//no more free memory :(
		return NULL;
	}
	if ((pageStackPtr & PAGEENTRIESINPAGEMASK) == PAGEENTRIESINPAGEMASK && !newPageStackDealloced) {
		//stack space becomes allocated page
		newPageStackDealloced = true;
		void *vaddr = (void*)((pageStackPtr+1) * PTRSIZE + PAGESTACKSTART);
		return getPhysPage(vaddr);
	}
	newPageStackDealloced = false;
	return curPageStack[pageStackPtr--];
}

void deallocPhysPage(void *page) {
	if ((pageStackPtr & PAGEENTRIESINPAGEMASK) == PAGEENTRIESINPAGEMASK && !newPageStackAlloced) {
		//deallocated page becomes more stack space
		newPageStackAlloced = true;
		void *vaddr = (void*)((pageStackPtr+1) * PTRSIZE + PAGESTACKSTART);
		mapPage(vaddr, page);
	} else {
		newPageStackAlloced = false;
		curPageStack[++pageStackPtr] = page;
	}
}

