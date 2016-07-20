#ifndef MEMORY_H
#define MEMORY_H

#define MEMORY_TABLE_ADDR 0x4000

struct memory_block {
	int address; //virtual address
	int type; //bit 0 clear = free to allocate
};

void movemem_test(void);
void init_memory(void);

void TLB_update(void);

void globalpages_setup(void);

int alloc_page(void);

void dealloc_page(int pageaddr);

void insert_in_memtable(struct memory_block *space, int index);
void delete_from_memtable(int index);


extern int memtable_size;
extern int page_stack_pointer;

#endif
