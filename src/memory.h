#ifndef MEMORY_H
#define MEMORY_H

#define MEMORY_TABLE_ADDR 0x4000

void init_memory(void);

void TLB_update(void);

void movemem(char *startaddr, int nrofbytes, int offset);

void globalpages_setup(void);

void alloc_page(int *pagetable_entry);

struct memory_block {
	int address; //virtual address
	int type; //bit 0 clear = free to allocate
};

extern int memtable_size;
extern int page_stack_pointer;

#endif
