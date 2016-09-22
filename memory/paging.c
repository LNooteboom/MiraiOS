#include <memory.h>

void *alloc_page(void) {
	current_page_stack->sp--;
	void *returnvalue = current_page_stack->free_pages[current_page_stack->sp];
	if (current_page_stack->sp == 0) {
		if (current_page_stack->next_page_stack == 0) {
			//out of memory!
			sprint("\nOut of memory!");
		}
		struct page_stack_page *old_page_stack = current_page_stack;
		current_page_stack = old_page_stack->next_page_stack;
		current_page_stack->free_pages[current_page_stack->sp] = old_page_stack;
		current_page_stack->sp++;
	}
	return returnvalue;
}

void dealloc_page(void *page) {
	if (current_page_stack->sp == 1023) {
		//allocate new page
		void* vmem = kernel_mem_stop;
		void* pmem = current_page_stack->free_pages[current_page_stack->sp - 1];
		set_in_kernel_pages(vmem, pmem);
		kernel_mem_stop += PAGE_SIZE;
		//struct page_stack_page *newstack = (struct page_stack_page*)(current_page_stack->free_pages[current_page_stack->sp - 1]);
		struct page_stack_page *newstack = (struct page_stack_page*)(vmem);
		current_page_stack->sp--;
		newstack->next_page_stack = (void*)current_page_stack;
		newstack->sp = 0;
		current_page_stack = newstack;
	}
	current_page_stack->free_pages[current_page_stack->sp/* - 1*/] = page; //this is where something goes wrong
	current_page_stack->sp++;
}

void set_in_kernel_pages(void *vmem, void *pmem) {
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
