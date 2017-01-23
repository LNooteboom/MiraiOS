#include <mm/physpaging.h>

#include <global.h>
#include <mm/pagemap.h>
#include <mm/paging.h>
#include <spinlock.h>
#include <print.h>
#include <mm/memset.h>

#define PAGE_SIZE				4096
#define LARGE_PAGE_SIZE 		(PAGE_SIZE * 512)

#define PAGE_STACK_LENGTH		(PAGE_SIZE / 8 - 1)

typedef physPage_t stackEntry_t;

struct pageStack {
	physPage_t prevStack;
	stackEntry_t pages[PAGE_STACK_LENGTH];
};

struct pageStackInfo {
	struct pageStack *stack;
	int32_t sp;
	spinlock_t lock;
};

static struct pageStackInfo smallPages;
static struct pageStackInfo largePages;
static struct pageStackInfo smallCleanPages;
static struct pageStackInfo largeCleanPages;

static uintptr_t freeBufferSmall;
static uintptr_t freeBufferLarge;

//struct pageStack firstStack[4] __attribute__((aligned(PAGE_SIZE)));


static stackEntry_t popPage(struct pageStackInfo *pages) {
	stackEntry_t newPage = NULL;
	acquireSpinlock(&(pages->lock));
	if (pages->sp >= 0 && pages->sp < PAGE_STACK_LENGTH - 1) {
		//pop a page off the stack
		newPage = pages->stack->pages[pages->sp];
		pages->sp--;
	} else if (pages->sp == -1) {
		//nothing is on the stack, use old stack space	
		newPage = mmGetPageEntry((uintptr_t)pages->stack);
		pages->sp--;
	} else if (pages->stack->prevStack) {
		//set new page stack
		mmMapPage((uintptr_t)pages->stack, pages->stack->prevStack, PAGE_FLAG_WRITE);
		//get new page
		newPage = pages->stack->pages[PAGE_STACK_LENGTH - 1];
		pages->sp = PAGE_STACK_LENGTH - 2;
	}
	releaseSpinlock(&(pages->lock));
	return newPage;
}

static void pushPage(struct pageStackInfo *pages, stackEntry_t newPage) {
	acquireSpinlock(&(pages->lock));
	if (pages->sp >= -1 && pages->sp < PAGE_STACK_LENGTH - 1) {
		//just push a new page on the stack
		pages->sp++;
		pages->stack->pages[pages->sp] = newPage;
	} else {
		//new page becomes more stack space
		physPage_t oldStack = mmGetPageEntry((uintptr_t)pages->stack);
		mmMapPage((uintptr_t)pages->stack, newPage, PAGE_FLAG_WRITE);\
		pages->stack->prevStack = oldStack;
		pages->sp = -1;
	}
	releaseSpinlock(&(pages->lock));
}


/*
Divides a large page into smaller pages
returns 0 if successful
*/
static uint8_t splitLargePage(struct pageStackInfo *largePages, struct pageStackInfo *smallPages) {
	physPage_t largePage = popPage(largePages);
	if (!largePage) {
		return 1;
	}
	for (uint16_t i = 0; i < (LARGE_PAGE_SIZE / PAGE_SIZE); i++) {
		deallocPhysPage(largePage);
		pushPage(smallPages, largePage);
		largePage += PAGE_SIZE;
	}
	return 0;
}




void mmInitPhysPaging(uintptr_t firstStack, uintptr_t freeMemBufferSmall, uintptr_t freeMemBufferLarge) {
	freeBufferSmall = freeMemBufferSmall;
	freeBufferLarge = freeMemBufferLarge;

	struct pageStack *stack = (struct pageStack*)(firstStack);
	smallPages.stack = &stack[0];
	smallPages.stack->prevStack = NULL;
	smallPages.sp = -1;
	
	largePages.stack = &stack[1];
	largePages.stack->prevStack = NULL;
	largePages.sp = -1;

	smallCleanPages.stack = &stack[2];
	smallCleanPages.stack->prevStack = NULL;
	smallCleanPages.sp = -1;

	largeCleanPages.stack = &stack[3];
	largeCleanPages.stack->prevStack = NULL;
	largeCleanPages.sp = -1;
}


physPage_t allocPhysPage(void) {
	physPage_t page = popPage(&smallPages);
	if (!page) {
		page = popPage(&smallCleanPages);
		if (!page) {
			if (splitLargePage(&largePages, &smallPages)) {
				if (splitLargePage(&largeCleanPages, &smallCleanPages)) {
					return NULL; //No more free memory
				} else {
					page = popPage(&smallCleanPages);
				}
			} else {
				page = popPage(&smallPages); //Possible race condition here, if all small pages are popped after a large one is split
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
			if (splitLargePage(&largeCleanPages, &smallCleanPages)) {
				if (splitLargePage(&largePages, &smallPages)) {
					return NULL;
				} else {
					page = popPage(&smallPages);
					mmMapPage(freeBufferSmall, page, PAGE_FLAG_WRITE);
					memset((void*)freeBufferSmall, 0, PAGE_SIZE);
				}
			} else {
				page = popPage(&smallCleanPages);
			}
		} else {
			mmMapPage(freeBufferSmall, page, PAGE_FLAG_WRITE);
			memset((void*)freeBufferSmall, 0, PAGE_SIZE);
		}
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
			return NULL;
		} else {
			mmMapLargePage(freeBufferLarge, page, PAGE_FLAG_WRITE);
			memset((void*)freeBufferLarge, 0, LARGE_PAGE_SIZE);
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
