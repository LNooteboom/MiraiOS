
void kmain(void) {
	init_memory();
	video_init();
	irq_init();
	initPICS();
	cursorX = partable->cursorX;
	cursorY = partable->cursorY;
	tty_set_full_screen_attrib(0x07);
	sprint("Kernel initialising...\n");

	ps2_init();
	PIT_init();
	init_memory_manager();

	sprint("Initialisation complete!\n");
	while (1) {};
}

