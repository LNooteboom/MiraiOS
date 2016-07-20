#include "memory.h"
#include "param.h"
#include "tty.h"

#define MEMTYPE_RESERVED 0x00
#define MEMTYPE_FREE 0x01
#define MEMTYPE_INUSE 0x02
#define MEMTYPE_ENDOFMEM 0x03

int memtable_size = 0;
int page_stack_pointer = 0;

void globalpages_setup(void) {
	struct memdetectentry *BIOSmemdetect = partable->memory_table;
	struct memory_block *page_table = (struct memory_block*) MEMORY_TABLE_ADDR;
	char psp_set = 0;
	for (int i = 0; i < partable->memtable_sz; i++) {
		page_table->address = BIOSmemdetect->baseLow;
		if (BIOSmemdetect->baseLow < 0x10000) {
			page_table->type = MEMTYPE_RESERVED; //mark as reserved
		} else if (BIOSmemdetect->type == 1) {
			page_table->type = MEMTYPE_FREE;
			if (psp_set == 0) {
				page_stack_pointer = i+1;
				psp_set = 1;
			}
		} else {
			page_table->type = MEMTYPE_RESERVED; //mark as reserved
		}
		BIOSmemdetect++;
		page_table++;
		memtable_size++;
	}
	//set end of memory
	page_table->address = (BIOSmemdetect->baseLow) + (BIOSmemdetect->lengthLow);
	page_table->type = MEMTYPE_ENDOFMEM; //mark as end of memory
}

int alloc_page(void) {
	int currentindex = page_stack_pointer;
	struct memory_block *freespace = (struct memory_block*)(MEMORY_TABLE_ADDR) + currentindex;
	struct memory_block *spaceafter = freespace+1; //space after free space
	struct memory_block *spacebefore = freespace-1;
	//freespace->address += 0x1000;
	if (spacebefore->type != MEMTYPE_INUSE) {
		struct memory_block newspace;
		newspace.address = freespace->address;
		newspace.type = MEMTYPE_INUSE;
		insert_in_memtable(&newspace, currentindex);
		page_stack_pointer++;
		freespace++;
		spaceafter++;
		spacebefore++;
	}
	while (freespace->address == spaceafter->address) {
		//jump over reserved/used space
		do {
			if (spaceafter->type == 3) {
				//out of memory
				sprint("Out of memory!", currentattrib);
				while(1){}
			}
			freespace++;
			spaceafter++;
			page_stack_pointer++;
		} while (freespace->type != 0x01); //find next free space
	}
	//found free page
	int return_value = (freespace->address) | 0x07;
	freespace->address += 0x1000;
	TLB_update();
	return return_value;
}
void dealloc_page(int pageaddr) {
	int currentindex = memtable_size - 1;
	struct memory_block *currentspace = (struct memory_block*)(MEMORY_TABLE_ADDR) + currentindex;
	while (currentspace->address > pageaddr) {
		currentspace--;
		currentindex--;
	}
	struct memory_block *spaceafter = currentspace+1; //space after free space
	struct memory_block *spacebefore = currentspace-1;
	
	if ((int)currentspace == pageaddr) { //if dealloc'd space is at the start of used space
		page_stack_pointer = currentindex;
		currentspace->address+=0x1000;
		if (currentspace->address == spaceafter->address) {
			delete_from_memtable(currentindex);
		}
	} else {
		struct memory_block newspace;
		newspace.address = pageaddr;
		newspace.type= MEMTYPE_FREE;
		currentindex++;
		insert_in_memtable(&newspace, currentindex);
		currentspace++;
		spaceafter++;
		spacebefore++;
		if (spaceafter->address != pageaddr + 0x1000) {
			//if dealloc'd space is not at the end of the used space:
			struct memory_block newspace;
			newspace.address = pageaddr + 0x1000;
			newspace.type= MEMTYPE_INUSE;
			currentindex++;
			insert_in_memtable(&newspace, currentindex);
		}
	}
}

void insert_in_memtable(struct memory_block *space, int index) {
	struct memory_block *source = (struct memory_block*)(MEMORY_TABLE_ADDR) + memtable_size - 1;
	struct memory_block *dest = source+1;
	for (int i = memtable_size; i > index; i--) {
		*dest = *source;
		source--;
		dest--;
	}
	memtable_size++;
	struct memory_block *newplace = (struct memory_block*)(MEMORY_TABLE_ADDR) + index;
	*newplace = *source;
}
void delete_from_memtable(int index) {
	struct memory_block *dest = (struct memory_block*)(MEMORY_TABLE_ADDR) + index;
	struct memory_block *source = dest+1;
	for (int i = index; i < memtable_size; i++) {
		*dest = *source;
		source++;
		dest++;
	}
	memtable_size--;
}
