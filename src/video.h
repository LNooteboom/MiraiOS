volatile char *vram = (volatile char*)0xB8000;

void write_to_vram(char value, int offset);

void video_init(void);

int get_line_width(void);

int vga_get_vertchars(void);

void vga_set_cursor(int cursorX, int cursorY);
