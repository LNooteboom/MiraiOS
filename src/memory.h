#ifndef MEMORY_H
#define MEMORY_H

#define MEMTABLE_1_ADDR 0xC0005000
#define PAGE_DIR	0xC0001000

#define MEM_BLOCK_TABLE_SIZE 340

//memory.asm

void init_memory(void);

void TLB_update(void);

//memory.c

struct page_stack_page {
	void *next_page_stack;
	int sp; //stack pointer
	int free_pages[1022];
};
struct mem_block {
	int address;
	int size;
	char metadata; //bit 0&1 = type, 00 = ignore 01 = free 10 =in-use
};
struct mem_block_table {
	void *next_mem_block_table;
	struct mem_block content[MEM_BLOCK_TABLE_SIZE];
};

void init_memory_manager(void);

void page_stack_setup(void);

int alloc_page(void);

void dealloc_page(int page);

void set_in_kernel_pages(int vmem, int pmem);

void mem_block_table_setup(void *destination, struct mem_block_table *prev_table);

void *alloc_mem(struct mem_block_table *table, int size);

#endif
