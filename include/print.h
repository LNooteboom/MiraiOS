#ifndef INCLUDE_PRINT_H
#define INCLUDE_PRINT_H

#include <global.h>

/*
Prints a character to the output console
*/
void cprint(char c);

/*
Prints a string to the output console
*/
void sprint(char*text);

/*
Prints a 32-bit integer as hex to the output console
*/
void hexprint(uint32_t value);

/*
Prints a 32-bit integer as hex to the output console
With newline
*/
void hexprintln(uint32_t value);

/*
Prints a 64-bit integer as hex to the output console
*/
void hexprint64(uint64_t value);

/*
Prints a 64-bit integer as hex to the output console
With newline
*/
void hexprintln64(uint64_t value);

/*
Prints a 32-bit integer as decimal to the output console
*/
void decprint(int32_t value);

/*
Prints a 32-bit integer as decimal to the output console
With newline
*/
void decprintln(int32_t value);

#endif
