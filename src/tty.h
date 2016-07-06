#ifndef TTY_H
#define TTY_H

extern int linewidth;
extern int screenheight;
extern int cursorX;
extern int cursorY;
extern char currentattrib;

void cprint(char c, char attrib);
void hexprint(int value, char attrib);
void newline(void);
void sprint(char *text, char attrib);

#endif
