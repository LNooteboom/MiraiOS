#include "memory.h"
#include "param.h"
#include "tty.h"

void globalpages_setup(void) {
	struct memdetectentry *memtable = partable->memory_table; //kernel parameter
	volatile struct memory_block *memtabledest = (volatile struct memory_block*) 0x4000;
	for (int i = 0; i < partable->memtable_sz; i++) {
		memtabledest->address = memtable->baseLow;
		memtabledest->size = memtable->lengthLow;
		memtabledest->type = memtable->type;

		if (memtable->baseLow < 0x10000 && memtable->type == 1) { //reserve lowmem
			//if (0x10000 - memtable->baseLow >= memtable->lenthLow) {
				memtabledest->type |= 0x80; //set in-use bit
			//}
		}
		sprint("Address: ", currentattrib);
		hexprint(memtabledest->address, currentattrib);
		sprint("\nSize: ", currentattrib);
		hexprint(memtabledest->size, currentattrib);
		sprint("\nType: ", currentattrib);
		hexprint(memtabledest->type, currentattrib);
		newline();
		newline();
		memtabledest++;
		memtable++;
	}
	
}
