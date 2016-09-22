#include "tty.h"
#include "video.h"
#include "ps2.h"

char scantable[] = {
0,0,0,0,0,0,0,0, //0-7
0,0,0,0,0,'	','`',0, //8-F
0,0,0,0,0,'q','1',0, //10-17
0,0,'z','s','a','w','2',0, //18-1F
0,'c','x','d','e','4','3',0, //20-27
0,' ','v','f','t','r','5',0, //28-2F
0,'n','b','h','g','y','6',0, //30-37
0,0,'m','j','u','7','8',0, //38-3F
0,',','k','i','o','0','9',0, //40-47
0,'.','/','l',';','p','-',0, //48-4F
0,0,'\'',0,'[','=',0,0, //50-57
0,0,0,']',0,'\\',0,0, //58-5F
0,0,0,0,0,0,0,0, //60-67
0,'1',0,'4','7',0,0,0, //68-6F
'0','.','2','5','6','8',0,0, //70-77
0,'+','3','-','*','9',0,0 //78-7F
};
char scantable_shift[] = {
0,0,0,0,0,0,0,0, //0-7
0,0,0,0,0,'	','~',0, //8-F
0,0,0,0,0,'Q','!',0, //10-17
0,0,'Z','S','A','W','@',0, //18-1F
0,'C','X','D','E','$','#',0, //20-27
0,' ','V','F','T','R','%',0, //28-2F
0,'N','B','H','G','Y','^',0, //30-37
0,0,'M','J','U','&','*',0, //38-3F
0,'<','K','I','O',')','(',0, //40-47
0,'>','?','L',':','P','_',0, //48-4F
0,0,'\"',0,'{','+',0,0, //50-57
0,0,0,'}',0,'|',0,0, //58-5F
0,0,0,0,0,0,0,0, //60-67
0,'1',0,'4','7',0,0,0, //68-6F
'0','.','2','5','6','8',0,0, //70-77
0,'+','3','-','*','9',0,0 //78-7F
};

char keybstate = 0;
// bit 0 = shift pressed

void presskey(char scancode) {
	char key;
	char dispchar;
	switch(scancode) {
		case (char)0xF0: //key released
		key = ps2_read_data();
		if (key == 0x12) {
			keybstate &= 0xFE;
		}
		break;

		case (char)0xE0: //multimedia keys
		key = ps2_read_data();
		switch (key) {
			case (char)0xF0:
			ps2_read_data();
			break;
			case (char)0x7D: //page up
			if (scrollY != 0) {
				vga_set_scroll(--scrollY);
			}
			break;
			case (char)0x75: //cursor up
			cursorY--;
			shift_cursor_left();
			vga_set_cursor(cursorX, cursorY);
			break;
			case (char)0x7A: //page down
			vga_set_scroll(++scrollY);
			break;
			case (char)0x72: //cursor down
			cursorY++;
			shift_cursor_left();
			vga_set_cursor(cursorX, cursorY);
			break;

			case (char)0x6B: //cursor left
			if (cursorX != 0) {
				cursorX--;
				vga_set_cursor(cursorX, cursorY);
			}
			break;
			case (char)0x74: //cursor right
			cursorX++;
			shift_cursor_left();
			vga_set_cursor(cursorX, cursorY);
			break;
		}
		break;

		case (char)0x66: //backspace
		backspace();
		break;
		case (char)0x5A: //enter
		newline();
		break;

		case (char)0x12: //left shift
		keybstate |= 1;
		break;

		default:
		if ((int) scancode < sizeof(scantable)) {
			if ((keybstate & 1) != 0) { //shift pressed
				dispchar = scantable_shift[(int) scancode];
			} else {
				dispchar = scantable[(int)scancode];
			}
			if (dispchar != 0) {
				cprint(dispchar);
			}
		}
		break;
	}
}

