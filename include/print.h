#ifndef INCLUDE_PRINT_H
#define INCLUDE_PRINT_H

#include <stdint.h>
#include <stdarg.h>

/*
Formatted print
*/
void printk(const char *fmt, ...);

void vprintk(const char *fmt, va_list args);

/*
Prints a character to the output console
*/
void putc(char c);

/*
Prints a string to the output console
*/
void puts(const char *text);

/*
Prints a 64-bit integer as hex to the output console
*/
void hexprint64(uint64_t value);

/*
Prints a 64-bit integer as hex to the output console
With newline
*/
void hexprintln64(uint64_t value);

#endif
