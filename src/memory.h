#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define MEMTABLE_1_ADDR 0xC0005000
#define PAGE_DIR	0xC0001000

#define PAGE_SIZE 0x1000

#define MEM_BLOCK_TABLE_SIZE 340

#define KERNEL_FREEMEM_SIZE (MEM_BLOCK_TABLE_SIZE * 0x1000)

#define KERNEL_HEAP_LIMIT 0x10000000

typedef int page_t;

//memory.asm

void init_memory(void);

void TLB_update(void);

//memory.c

extern struct mem_block_table *current_mem_block_table;

struct page_stack_page {
	void *next_page_stack;
	int sp; //stack pointer
	void *free_pages[1022];
};
struct mem_block {
	void* address;
	size_t size;
	char metadata; //bit 0&1 = type, 00 = ignore 01 = free 10 =in-use
};
struct mem_block_table {
	void *next_mem_block_table;
	struct mem_block content[MEM_BLOCK_TABLE_SIZE];
};

void init_memory_manager(void);

void page_stack_setup(void);

void *alloc_page(void);

void dealloc_page(void *page);

void set_in_kernel_pages(void *vmem, void *pmem);

void mem_block_table_setup(void *destination, struct mem_block_table *prev_table);

void *alloc_mem(struct mem_block_table *table, size_t size);

void dealloc_mem(struct mem_block_table *table, void *memptr);

#endif
