#include <mm/physpaging.h>

#include <stdint.h>
#include <stddef.h>
#include <mm/pagemap.h>
#include <mm/paging.h>
#include <sched/spinlock.h>
#include <print.h>
#include <mm/memset.h>

#include <arch/tlb.h> //evil!

//extern uint64_t *mmGetEntry(uintptr_t addr, char level);

#define PAGE_STACK_LENGTH		(PAGE_SIZE / 8 - 1)

typedef physPage_t stackEntry_t;

struct pageStack {
	physPage_t prevStack;
	stackEntry_t pages[PAGE_STACK_LENGTH];
};

struct pageStackInfo {
	struct pageStack *stack;
	int32_t sp;
	uint64_t nrofPages;
	spinlock_t lock;
};

static struct pageStackInfo smallPages;
static struct pageStackInfo largePages;
static struct pageStackInfo smallCleanPages;
static struct pageStackInfo largeCleanPages;

static uintptr_t freeBufferSmall;
static spinlock_t freeBufferSmallLock;
static uintptr_t freeBufferLarge;
static spinlock_t freeBufferLargeLock;

//struct pageStack firstStack[4] __attribute__((aligned(PAGE_SIZE)));

static stackEntry_t popPage(struct pageStackInfo *pages) {
	stackEntry_t newPage = 0;
	acquireSpinlock(&(pages->lock));
	if (pages->sp >= 0 && pages->sp < PAGE_STACK_LENGTH - 1) {
		//pop a page off the stack
		newPage = pages->stack->pages[pages->sp];
		pages->sp--;
		pages->nrofPages--;
	} else if (pages->sp == -1) {
		//nothing is on the stack, use old stack space	
		newPage = mmGetPageEntry((uintptr_t)pages->stack);
		if (pages->stack->prevStack) {
			//set new page stack
			mmMapPage((uintptr_t)pages->stack, pages->stack->prevStack, PAGE_FLAG_WRITE);
			pages->sp = PAGE_STACK_LENGTH - 1;
			pages->nrofPages--;

			releaseSpinlock(&(pages->lock));
			//invalidate tlb
			tlbInvalidateGlobal(pages->stack, 1);
			return newPage | 1;
		} else {
			pages->sp = -2;
		}
		newPage |= 1;
	} else {
		pages->sp = -2;
	}
	releaseSpinlock(&(pages->lock));
	return newPage;
}

static void pushPage(struct pageStackInfo *pages, stackEntry_t newPage) {
	acquireSpinlock(&(pages->lock));
	pages->nrofPages++;
	if (pages->sp >= -1 && pages->sp < PAGE_STACK_LENGTH - 1) {
		//just push a new page on the stack
		pages->sp++;
		pages->stack->pages[pages->sp] = newPage;
	} else {
		//new page becomes more stack space
		physPage_t oldStack = 0;
		if (pages->sp != -2) {
			oldStack = mmGetPageEntry((uintptr_t)pages->stack);
		}
		mmMapPage((uintptr_t)pages->stack, newPage, PAGE_FLAG_WRITE);
		//invalidate tlb
		tlbInvalidateGlobal(pages->stack, 1);

		pages->stack->prevStack = oldStack;
		pages->sp = -1;
	}
	releaseSpinlock(&(pages->lock));
}


/*
Divides a large page into smaller pages
returns the first small page and pushes the rest on the page stack
returns the first page != 0 if successful
*/
static physPage_t splitLargePage(struct pageStackInfo *_largePages, struct pageStackInfo *_smallPages) {
	physPage_t largePage = popPage(_largePages);
	if (!largePage) {
		return 0;
	}
	for (physPage_t i = PAGE_SIZE; i < (LARGE_PAGE_SIZE); i += PAGE_SIZE) {
		pushPage(_smallPages, largePage + i);
	}
	return largePage;
}

static void cleanSmallPage(physPage_t page) {
	acquireSpinlock(&freeBufferSmallLock);
	mmMapPage(freeBufferSmall, page, PAGE_FLAG_WRITE);
	tlbInvalidateLocal((void*)freeBufferSmall, 1);
	memset((void*)freeBufferSmall, 0, PAGE_SIZE);
	releaseSpinlock(&freeBufferSmallLock);
}

void mmInitPhysPaging(uintptr_t firstStack, uintptr_t freeMemBufferSmall, uintptr_t freeMemBufferLarge) {
	freeBufferSmall = freeMemBufferSmall;
	freeBufferLarge = freeMemBufferLarge;

	smallPages.stack = (struct pageStack*)(firstStack);
	smallPages.sp = -2;
	
	largePages.stack = (struct pageStack*)(firstStack + PAGE_SIZE);
	largePages.sp = -2;

	smallCleanPages.stack = (struct pageStack*)(firstStack + PAGE_SIZE * 2);
	smallCleanPages.sp = -2;

	largeCleanPages.stack = (struct pageStack*)(firstStack + PAGE_SIZE * 3);
	largeCleanPages.sp = -2;
}


physPage_t allocPhysPage(void) {
	physPage_t page = popPage(&smallPages);
	if (!page) {
		page = popPage(&smallCleanPages);
		if (!page) {
			page = splitLargePage(&largePages, &smallPages);
			if (!page) {
				page = splitLargePage(&largeCleanPages, &smallCleanPages);
				//page will be 0 if no more memory is available
			}
		}
	}
	return page;
}


physPage_t allocCleanPhysPage(void) {
	physPage_t page = popPage(&smallCleanPages);
	if (!page) {
		page = popPage(&smallPages);
		if (!page) {
			page = splitLargePage(&largeCleanPages, &smallCleanPages);
			if (!page) {
				page = splitLargePage(&largePages, &smallPages);
				if (!page) {
					return 0;
				} else {
					cleanSmallPage(page);
				}
			}
		} else {
			cleanSmallPage(page);
		}
	} else if (page & 1) {
		page &= ~1;
		cleanSmallPage(page);
	}
	return page;
}

physPage_t allocLargePhysPage(void) {
	physPage_t page = popPage(&largePages);
	if (!page) {
		page = popPage(&largeCleanPages);
	}
	return page;
}

physPage_t allocLargeCleanPhysPage(void) {
	uintptr_t page = popPage(&largeCleanPages);
	if (!page) {
		page = popPage(&largePages);
		if (!page) {
			return 0;
		} else {
			acquireSpinlock(&freeBufferLargeLock);
			mmMapLargePage(freeBufferLarge, page, PAGE_FLAG_WRITE);
			tlbInvalidateLocal((void*)freeBufferLarge, LARGE_PAGE_SIZE / PAGE_SIZE);
			memset((void*)freeBufferLarge, 0, LARGE_PAGE_SIZE);
			releaseSpinlock(&freeBufferLargeLock);
		}
	}
	return page;
}


void deallocPhysPage(physPage_t page) {
	pushPage(&smallPages, page);
}

void deallocLargePhysPage(physPage_t page) {
	pushPage(&largePages, page);
}

uint64_t getNrofPages(void) {
	return smallPages.nrofPages + smallCleanPages.nrofPages + largePages.nrofPages * (LARGE_PAGE_SIZE / PAGE_SIZE);
}