#include "memory.h"
#include "types.h"
#include "param.h"
#include "tty.h"

struct page_stack_page *current_page_stack;

void *kernel_mem_stop = (void*)0xC0110000;

struct freemem_marker *first_freemem;
struct freemem_marker *last_freemem;

void init_memory_manager(void) {
	page_stack_setup();
}

void page_stack_setup(void) {
	//start by assigning a pointer
	current_page_stack = (struct page_stack_page*)(MEMTABLE_1_ADDR);
	current_page_stack->next_page_stack = (void*)0;
	current_page_stack->sp = 0;
	//now we have to populate it so get the BIOS memory detect parameter
	struct memdetectentry *memDetect = (struct memdetectentry*)(partable->memory_table + 0xC0000000);
	for (int i = 0; i < partable->memtable_sz; i++) {
		if (memDetect->type == 0x01 /*&& memDetect->baseLow >= 0x100000*/) {
			//found free space
			void *currentpage = (void*)memDetect->baseLow;
			while ((int)currentpage < memDetect->lengthLow) {//something goes wrong here
				if ((int)currentpage > partable->krnlmemsize + 0x100000) {
					dealloc_page(currentpage);
				}
				currentpage += PAGE_SIZE;
			}
		}
		memDetect++;
	}
}

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

void *ksbrk(size_t size) {
	if (size & (PAGE_SIZE - 1)) {
		size &= ~(PAGE_SIZE - 1);
		size += PAGE_SIZE;
	}
	void *old_kmem_stop = kernel_mem_stop;
	for (int i = size; i > 0; i -= PAGE_SIZE) {
		set_in_kernel_pages(kernel_mem_stop, alloc_page());
		kernel_mem_stop += PAGE_SIZE;
	}
	add_freemem_marker(old_kmem_stop, (struct freemem_marker*)0, size);
	last_freemem->next = (struct freemem_marker*) old_kmem_stop;
	return old_kmem_stop;
}

void add_freemem_marker(void *pos, struct freemem_marker *next, size_t size) {
	struct freemem_marker *marker = (struct freemem_marker*)pos;
	marker->magic[0] = 'F';
	marker->magic[1] = 'R';
	marker->magic[2] = 'E';
	marker->magic[3] = 'E';
	marker->next = next;
	marker->size = size;
}

void *alloc_mem(struct mem_block_table *table, size_t size) {

}

void dealloc_mem(struct mem_block_table *table, void *memptr) {
	
}
