#ifndef INCLUDE_PRINT_H
#define INCLUDE_PRINT_H

#include <global.h>

void cprint(char c);

void sprint(char*text);

void hexprint(uint32_t value);
void hexprintln(uint32_t value);

void decprint(int32_t value);
void decprintln(int32_t value);

#endif
