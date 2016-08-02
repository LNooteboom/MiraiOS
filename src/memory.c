#include "memory.h"
#include "param.h"
#include "tty.h"

struct page_stack_page *current_page_stack;

int kernel_mem_end = 0xC0100000;

void page_stack_setup(void) {
	kernel_mem_end += partable->krnlmemsize;
	if ((kernel_mem_end & 0x0FFF) != 0) { //round off to pages
		kernel_mem_end &= 0xFFFFF000;
		kernel_mem_end += 0x00001000;
	}
	//start by assigning a pointer
	current_page_stack = (struct page_stack_page*)(MEMTABLE_1_ADDR);
	current_page_stack->next_page_stack = (void*)0;
	current_page_stack->sp = 0;
	//now we have to populate it so get the BIOS memory detect parameter
	struct memdetectentry *memDetect = (struct memdetectentry*)(partable->memory_table + 0xC0000000);
	for (int i = 0; i < partable->memtable_sz; i++) {
		if (memDetect->type == 0x01 /*&& memDetect->baseLow >= 0x100000*/) {
			//found free space
			int currentpage = memDetect->baseLow;
			while (currentpage < memDetect->lengthLow) {
				if (currentpage > partable->krnlmemsize + 0x100000) {
					dealloc_page(currentpage);
				}
				currentpage += 0x1000;
			}
		}
		memDetect++;
	}
	while(1);
	hexprint((int) current_page_stack, currentattrib);
	newline();
	hexprint(kernel_mem_end, currentattrib);
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
		int vmem = kernel_mem_end;
		int pmem = current_page_stack->free_pages[current_page_stack->sp - 1];
		set_in_kernel_pages(vmem, pmem);
		kernel_mem_end += 0x1000;
		//struct page_stack_page *newstack = (struct page_stack_page*)(current_page_stack->free_pages[current_page_stack->sp - 1]);
		struct page_stack_page *newstack = (struct page_stack_page*)(vmem);
		current_page_stack->sp--;
		newstack->next_page_stack = (void*)current_page_stack;
		newstack->sp = 0;
		current_page_stack = newstack;
	}
	current_page_stack->sp++;
	current_page_stack->free_pages[current_page_stack->sp - 1] = page;
	//hexprint(current_page_stack->sp, currentattrib);
}

void set_in_kernel_pages(int vmem, int pmem) {
	sprint("Kernel page add: vmem: ", currentattrib);
	hexprint(vmem, currentattrib);
	sprint(" pmem: ", currentattrib);
	hexprint(pmem, currentattrib);
	newline();
	if ((vmem & 0xFF800000) != 0xC0000000) {
		//for this we need to create a second table
		panic("Out of kernel address space!", (int)set_in_kernel_pages, 0);
		while(1){};
	}
	int *page_entry = (int*)(((vmem & 0x007FF000) >> 12) + PAGE_DIR);
	pmem &= 0xFFFFF000;
	*page_entry = pmem | 0x03; //flags
	while(1);
}
