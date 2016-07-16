#ifndef PARAM_H
#define PARAM_H

struct paramtable {
	int krnloffset;
	int krnlmemsize;

	char cursorX;
	char cursorY;

	memdetectentry memory_table[];
};
struct memdetectentry {
	int baseLow;
	int baseHigh;
	int lengthLow;
	int lengthHigh;
	int type;
	int unused;
};

volatile struct paramtable *partable = (volatile struct paramtable*)0x3000;

#endif
