
struct paramtable {
	int krnloffset;
	int krnlmemsize;

	char cursorX;
	char cursorY;
};

volatile struct paramtable *partable = (volatile struct paramtable*)0x3000;

