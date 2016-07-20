#include "memory.h"
#include "param.h"
#include "tty.h"

struct page_stack_page *current_page_stack;

void page_stack_setup(void) {
	//start by assigning a pointer
	current_page_stack = (struct page_stack_page*)(MEMTABLE_1_ADDR);
	current_page_stack->next_page_stack = (void*)0;
	current_page_stack->sp = 0;
	//now we have to populate it so get the BIOS memory detect parameter
	struct memdetectentry *memDetect = partable->memory_table;
	for (int i = 0; i < partable->memtable_sz; i++) {
		if (memDetect->type == 0x01 && memDetect->baseLow >= 0x100000) {
			//found free space
			int currentpage = memDetect->baseLow;
			while (currentpage < memDetect->lengthLow) {
				dealloc_page(currentpage);
				currentpage += 0x1000;
			}
		}
		memDetect++;
	}
	hexprint((int) current_page_stack, currentattrib);
}

int alloc_page(void) {
	current_page_stack->sp--;
	int returnvalue = current_page_stack->free_pages[current_page_stack->sp];
	if (current_page_stack->sp == 0) {
		if (current_page_stack->next_page_stack == 0) {
			//out of memory!
			sprint("\nOut of memory!", currentattrib);
		}
		struct page_stack_page *old_page_stack = current_page_stack;
		current_page_stack = old_page_stack->next_page_stack;
		current_page_stack->sp++;
		current_page_stack->free_pages[current_page_stack->sp - 1] = (int)old_page_stack;
	}
	return returnvalue;
}

void dealloc_page(int page) {
	if (current_page_stack->sp == 1023) {
		//allocate new page
		struct page_stack_page *newstack = (struct page_stack_page*)(current_page_stack->free_pages[current_page_stack->sp - 1]);
		current_page_stack->sp--;
		newstack->next_page_stack = (void*)current_page_stack;
		newstack->sp = 0;
		current_page_stack = newstack;
	}
	current_page_stack->sp++;
	current_page_stack->free_pages[current_page_stack->sp - 1] = page;
}

