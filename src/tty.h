#ifndef TTY_H
#define TTY_H

extern int cursorX;
extern int cursorY;
extern char currentattrib;

void cprint(char c, char attrib);
void hexprint(int value, char attrib);
void newline(void);
void backspace(void);
void shift_cursor_left(void);
void sprint(char *text, char attrib);

void tty_set_full_screen_attrib(char attrib);
void tty_clear_screen(void);

void panic(char *msg, int addr, char errorcode);

#endif
