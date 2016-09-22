#ifndef TTY_H
#define TTY_H

extern int cursorX;
extern int cursorY;
extern char currentattrib;

void cprint(char c);

void hexprint(int value);
void hexprintln(int value);
void decprint(int value);
void decprintln(int value);

void newline(void);
void backspace(void);
void shift_cursor_left(void);
void sprint(char *text);

void tty_set_full_screen_attrib(char attrib);
void tty_clear_screen(void);

void panic(char *msg, int addr, char errorcode);

#endif
