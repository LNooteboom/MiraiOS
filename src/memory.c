#include "memory.h"
#include "param.h"
#include "tty.h"

struct page_stack_page *current_page_stack;

struct mem_block_table *current_mem_block_table;

int kernel_mem_end = 0xC0110000;

void init_memory_manager(void) {
	page_stack_setup();
	asm ("xchgw %bx, %bx");
	set_in_kernel_pages(kernel_mem_end, alloc_page());
	current_mem_block_table = (struct mem_block_table*)kernel_mem_end;
	mem_block_table_setup((void*)kernel_mem_end, (struct mem_block_table*)0);
	sprint("current mem block table: ");
	hexprint((int) current_mem_block_table);
	newline();
	sprint("kernel mem end: ");
	hexprint(kernel_mem_end);
	newline();
	kernel_mem_end += 0x1000;

	current_mem_block_table->content[0].address = kernel_mem_end;
	current_mem_block_table->content[0].size = 0xFFFFFFFF - kernel_mem_end;
	current_mem_block_table->content[0].metadata = 1; //mark as free
	while(1);
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
			int currentpage = memDetect->baseLow;
			while (currentpage < memDetect->lengthLow) {//something goes wrong here
				if (currentpage > partable->krnlmemsize + 0x100000) {
					dealloc_page(currentpage);
				}
				currentpage += 0x1000;
			}
		}
		memDetect++;
	}
}

int alloc_page(void) {
	current_page_stack->sp--;
	int returnvalue = current_page_stack->free_pages[current_page_stack->sp];
	if (current_page_stack->sp == 0) {
		if (current_page_stack->next_page_stack == 0) {
			//out of memory!
			sprint("\nOut of memory!");
		}
		struct page_stack_page *old_page_stack = current_page_stack;
		current_page_stack = old_page_stack->next_page_stack;
		current_page_stack->free_pages[current_page_stack->sp] = (int)old_page_stack;
		current_page_stack->sp++;
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
	current_page_stack->free_pages[current_page_stack->sp/* - 1*/] = page; //this is where something goes wrong
	current_page_stack->sp++;
}

void set_in_kernel_pages(int vmem, int pmem) {
	if ((vmem & 0xFF800000) != 0xC0000000) {
		//for this we need to create a second table
		panic("Out of kernel address space!", (int)set_in_kernel_pages, 0);
		while(1){};
	}
	int *page_entry = (int*)(((vmem & 0x007FF000) >> 12) + PAGE_DIR);
	pmem &= 0xFFFFF000;
	*page_entry = pmem | 0x03; //flags
	//asm volatile ("xchgw %bx, %bx"); //magic breakpoint
	TLB_update();
}

void mem_block_table_setup(void *destination, struct mem_block_table *prev_table) {
	if (((int)destination & 0xFFF) != 0) {
		sprint("WARN: please align memory block table to 4kb!");
	}
	if (prev_table != 0) {
		prev_table->next_mem_block_table = (struct mem_block_table*)destination;
	}

	struct mem_block_table *table = (struct mem_block_table*)destination;
	table->next_mem_block_table = 0;
	for (int i = 0; i < MEM_BLOCK_TABLE_SIZE; i++) {
		table->content[i].metadata = 0;
	}
}

void *alloc_mem(struct mem_block_table *table, int size) {
	struct mem_block *free_block;
	struct mem_block *new_block;
	char blocks_found = 0;
	//find first free block
	for (int i = 0; i < MEM_BLOCK_TABLE_SIZE; i++) {
		if (table->content[i].metadata == 1 && (blocks_found & 1) == 0) {
			//found free page
			free_block = (table->content + i);
			blocks_found |= 1;
		} else if (table->content[i].metadata == 0 && (blocks_found & 2) == 0) {
			//found empty entry
			new_block = (table->content + i);
			blocks_found |= 2;
		}
		if (blocks_found == 3) {
			break;
		}
		if (i == MEM_BLOCK_TABLE_SIZE - 1) {
			//search next block
			if (table->next_mem_block_table != 0) {
				table = (struct mem_block_table*)(table->next_mem_block_table);
			} else {
				//create new table
				//TODO
				sprint("TODO: create new table!!");
				while(1);
			}
		}
	}
	new_block->metadata = 2;
	new_block->address = free_block->address;
	new_block->size = size;
	free_block->address += size;
	free_block->size -= size;

	return (void*)(new_block->address);
}
