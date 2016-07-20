#ifndef MEMORY_H
#define MEMORY_H

#define MEMTABLE_1_ADDR 0x4000

//memory.asm

void init_memory(void);

void TLB_update(void);

//memory.c

struct page_stack_page {
	void *next_page_stack;
	int sp; //stack pointer
	int free_pages[1022];
};

void page_stack_setup(void);

int alloc_page(void);

void dealloc_page(int page);

#endif
