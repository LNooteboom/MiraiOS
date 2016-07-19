#ifndef MEMORY_H
#define MEMORY_H

void init_memory(void);

void alloc_page_to_kernel(int page);

void globalpages_setup(void);

struct memory_block {
	int address; //virtual address
	int size;
	int type; //bit 0 clear = free to allocate
};

#endif
