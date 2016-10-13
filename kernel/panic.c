
void panic(char *msg, int addr, char errorcode) {
	currentattrib = 0x1F;
	vga_set_scroll(0);
	tty_clear_screen();
	tty_set_full_screen_attrib(currentattrib);
	cursorX = 0;
	cursorY = 0;
	sprint(msg);
	newline();
	sprint("At ");
	hexprintln(addr);
	if (errorcode) {
		sprint("Error code: ");
		hexprint(errorcode);
		newline();
	}
}
