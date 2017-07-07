#ifndef INCLUDE_PANIC_H
#define INCLUDE_PANIC_H

/*
Notify the user something terrible has happened
*/
void __attribute__((noreturn)) panic(const char *fmt, ...);

#endif