#ifndef PARAM_H
#define PARAM_H

struct paramtable {
	int krnloffset;
	int krnlmemsize;

	char cursorX;
	char cursorY;

	short memtable_sz;

	//struct memdetectentry *memory_table;
	int memory_table;
};

struct memdetectentry {
	int baseLow;
	int baseHigh;
	int lengthLow;
	int lengthHigh;
	int type;
	int unused;
};

extern struct paramtable *partable;
#endif
