#ifndef TTY_H
#define TTY_H

#include <stdint.h>

const char *execCommand(const char *command);

void newline(void);

void backspace(void);

void cursorLeft(void);

void setFullScreenColor(char attrib);

void clearScreen(void);

#endif
