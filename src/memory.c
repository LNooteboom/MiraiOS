#include "memory.h"
#include "param.h"
#include "tty.h"


int memtable_size = 0;
int page_stack_pointer = 0;

void globalpages_setup(void) {
	struct memdetectentry *BIOSmemdetect = partable->memory_table;
	struct memory_block *page_table = (struct memory_block*) MEMORY_TABLE_ADDR;
	char psp_set = 0;
	for (int i = 0; i < partable->memtable_sz; i++) {
		page_table->address = BIOSmemdetect->baseLow;
		if (BIOSmemdetect->baseLow < 0x10000) {
			page_table->type = 0x00; //mark as reserved
		} else if (BIOSmemdetect->type == 1) {
			page_table->type = 0x01; //mark as free to use
			if (psp_set == 0) {
				page_stack_pointer = i;
				psp_set = 1;
				//test:
				page_table++;
				memtable_size++;
				page_table->address = 0x101000;
				page_table->type = 0x00;
				page_table++;
				memtable_size++;
				page_table->address = 0x102000;
				page_table->type = 0x01;
			}
		} else {
			page_table->type = 0x00; //mark as reserved
		}
		hexprint(page_table->address, currentattrib);
		newline();
		hexprint(page_table->type, currentattrib);
		newline();
		newline();
		BIOSmemdetect++;
		page_table++;
		memtable_size++;
	}
	//set end of memory
	page_table->address = (BIOSmemdetect->baseLow) + (BIOSmemdetect->lengthLow);
	page_table->type = 0x03; //mark as end of memory
}

void alloc_page(int *pagetable_entry) {
	struct memory_block *freespace = (struct memory_block*)((page_stack_pointer * 8) + MEMORY_TABLE_ADDR);
	struct memory_block *space2 = freespace+1; //space after free space
	while (freespace->address == space2->address) {
		//jump over reserved/used space
		do {
			if (space2->type == 3) {
				//out of memory
				sprint("Out of memory!", currentattrib);
				while(1){}
			}
			freespace++;
			space2++;
		} while (freespace->type != 0x01); //find next free space
	}
	//found free page
	*pagetable_entry = (freespace->address) | 0x07;
	freespace->address += 0x1000;
	TLB_update();
}
