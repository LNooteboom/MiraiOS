#include "memory.h"
#include "types.h"
#include "param.h"
#include "tty.h"

struct page_stack_page *current_page_stack;

struct mem_block_table *current_mem_block_table;

void *kernel_heap_end = (void*)0xC0110000;

void init_memory_manager(void) {
	page_stack_setup();
	//asm ("xchgw %bx, %bx");
	set_in_kernel_pages(kernel_heap_end, alloc_page());
	current_mem_block_table = (struct mem_block_table*)kernel_heap_end;
	mem_block_table_setup((void*)kernel_heap_end, (struct mem_block_table*)0);
	kernel_heap_end += 0x1000;

	current_mem_block_table->content[0].address = 0;
	current_mem_block_table->content[0].size = (size_t)kernel_heap_end;
	current_mem_block_table->content[0].metadata = 2; //mark as used

	current_mem_block_table->content[1].address = kernel_heap_end;
	current_mem_block_table->content[1].size = KERNEL_FREEMEM_SIZE;
	current_mem_block_table->content[1].metadata = 1; //mark as free
	//while(1);
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
				currentpage += 0x1000;
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
		void* vmem = kernel_heap_end;
		void* pmem = current_page_stack->free_pages[current_page_stack->sp - 1];
		set_in_kernel_pages(vmem, pmem);
		kernel_heap_end += 0x1000;
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
void *sbrk(size_t size) {
	void *start = kernel_heap_end;
	if (size & 0xFFF) {
		size &= 0xFFF;
		size += PAGE_SIZE;
	}
	for (int i = 0; i < size / PAGE_SIZE; i++) {
		if ((int)kernel_heap_end >= KERNEL_HEAP_LIMIT) {
			sprint("Out of kernel heap space!");
			size = i * PAGE_SIZE;
			break;
		}
		set_in_kernel_pages(kernel_heap_end, alloc_page());
		kernel_heap_end += PAGE_SIZE;
	}
	//extend free space
	struct mem_block_table *table = current_mem_block_table;
	bool unused_block_found = false;
	struct mem_block *unused_space;
	for (int i = 0; i < MEM_BLOCK_TABLE_SIZE; i++) {
		struct mem_block *entry = table->content + i;
		if (entry->metadata == 1 && (void*)(entry->address + entry->size) == start) {
			entry->size += size;
			break;
		} else if (entry->metadata == 0) {
			unused_space = entry;
			unused_block_found = true;
		}
		if (i == MEM_BLOCK_TABLE_SIZE - 1) {
			if (table->next_mem_block_table != 0) {
				table = table->next_mem_block_table;
				i = 0;
			} else {
				if (!unused_block_found) {
					sprint("TODO: create new table");
					while(true);
				}
				unused_space->address = start;
				unused_space->size = size;
				unused_space->metadata = 1;
				break;
			}
		}
	}
	return start;
	
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
char page_is_present(int vmem) {
	if ((vmem & 0xFF800000) != 0xC0000000) {
		sprint("Trying to get page index out of table!");
		while(1);
	}
	int *page_entry = (int*)(((vmem & 0x007FF000) >> 12) + PAGE_DIR);
	return (char)(*page_entry & 1);
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

void *alloc_mem(struct mem_block_table *table, size_t size) {
	struct mem_block *free_block;
	struct mem_block *new_block;
	char blocks_found = 0;
	//find first free block
	for (int i = 0; i < MEM_BLOCK_TABLE_SIZE; i++) {
		if (table->content[i].metadata == 1 && (blocks_found & 1) == 0 && table->content[i].size >= size) {
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
				i = 0;
			} else {
				if ((blocks_found & 1) == 0) {
					//create new table
					//TODO
					sprint("TODO: create new table!!");
					while(1);
				} else {
					sprint("out of kernel vmem!");
					while(1);
				}
			}
		}
	}
	new_block->metadata = 2;
	new_block->address = free_block->address;
	new_block->size = size;
	free_block->address += size;
	free_block->size -= size;

	if (free_block->size == 0) {
		free_block->metadata = 0;
	}

	return (void*)(new_block->address);
}

void dealloc_mem(struct mem_block_table *table, void *memptr) {
	struct mem_block_table *old_table = table;
	struct mem_block *block_current;
	for (int i = 0; i < MEM_BLOCK_TABLE_SIZE; i++) {
		if (table->content[i].address == memptr && table->content[i].metadata == 2) {
			block_current = &table->content[i];
			break;
		}
		if (i == (MEM_BLOCK_TABLE_SIZE - 1)) {
			if (table->next_mem_block_table != 0) {
				table = (struct mem_block_table*)(table->next_mem_block_table);
				i = 0;
			} else {
				sprint("Could not find memory block or memory block is already freed!\n");
				//while(1);
				return;
			}
		}
	}
	block_current->metadata = 1;
	char blocks_found = 0;
	struct mem_block *block_lower;
	struct mem_block *block_higher;
	table = old_table;

	for (int i = 0; i < MEM_BLOCK_TABLE_SIZE; i++) {
		if ((blocks_found & 1) == 0 && table->content[i].address + table->content[i].size == block_current->address) {
			block_lower = &table->content[i];
			blocks_found |= 1;
		} else if ((blocks_found & 2) == 0 && table->content[i].address == (block_current->address + block_current->size)) {
			block_higher = &table->content[i];
			blocks_found |= 2;
		}
		if (blocks_found == 3) {
			break;
		}
		if (i == (MEM_BLOCK_TABLE_SIZE - 1) && table->next_mem_block_table != 0) {
			table = (struct mem_block_table*)(table->next_mem_block_table);
			i = 0;
		}
	}

	if ((blocks_found & 1) != 0 && block_lower->metadata == 1) {
		block_lower->size += block_current->size;
		block_current->metadata = 0;
		block_current = block_lower;
	}
	if ((blocks_found & 2) != 0 && block_higher->metadata == 1) {
		block_current->size += block_higher->size;
		block_higher->metadata = 0;
	}
}
