void write_to_vram(char value, unsigned short offset);
void getCRTCPorts(void);
int get_line_width(void);
char inb(short port);
void outb(short port, char value);
void cprint(char c, char attrib);
void hexprint(int value);

int linewidth;
int cursorX = 0;
int cursorY = 0;

void kmain(void) {
	getCRTCPorts();
	linewidth = get_line_width();
	hexprint(linewidth);
	write_to_vram('%', linewidth);
	//do nothing for now
	while (1) {};
}

void cprint(char c, char attrib) {
	unsigned short offset = (cursorY * 100) + (cursorX << 1);
	write_to_vram(c, offset);
	offset++;
	write_to_vram(attrib, offset);

	cursorX++;
	if (cursorX > linewidth) {
		cursorY++;
		cursorX = 0;
	}
}
void hexprint(int value) {
	for (int i = 7; i >= 0; i--) {
		char currentnibble = (value >> (i << 2)) & 0x0F;
		if (currentnibble < 10) {
			//0-9
			currentnibble += '0';
		} else {
			currentnibble += 'A' - 10;
		}
		cprint(currentnibble, 0x07);
	}
}
