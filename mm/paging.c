#include <paging.h>
#include "paging.h"

pageStack_t *curPageStack;
int16_t pageStackPtr;


void *allocPhysPage(void) {
	void *returnValue = curPageStack->pages[pageStackPtr--];
	if (pageStackPtr < 0) {
		if (curPageStack->next == NULL) {
			panic("Out of memory!");
			return NULL;
		}
		void *oldPageStack = (void*) curPageStack;
		curPageStack = curPageStack->next;
		pageStackPtr = NROFPAGESINSTACK - 1;

		//dealloc the old page stack
		curPageStack->pages[pageStackPtr] = oldPageStack;
	}
}

void deallocPhysPage(void *page) {
	if (pageStackPtr >= NROFPAGESINSTACK - 1) {
		//deallocated page becomes new page stack
		pageStack_t newStack = (pageStack_t*) page;
		newStack->next = curPageStack;
		pageStackPtr = 0;
	}
}

void setInKernelPages(void *vmem, void *pmem) {
	if (( (int)vmem & 0xFF800000) != 0xC0000000) {
		//for this we need to create a second table
		panic("Out of kernel address space!", (int)set_in_kernel_pages, 0);
		while(1){};
	}
	void **page_entry = (void**)((( (int)vmem & 0x007FF000) >> 12) + PAGE_DIR);
	pmem = (void*)((int)pmem & 0xFFFFF000);
	*page_entry = (void*)( (int)pmem | 0x03); //flags
	//asm volatile ("xchgw %bx, %bx"); //magic breakpoint
	TLB_update();
}
