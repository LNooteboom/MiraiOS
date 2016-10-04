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
