#ifndef INCLUDE_PANIC_H
#define INCLUDE_PANIC_H

void __attribute__((noreturn)) die(void *arg);

/*
Notify the user something terrible has happened
*/
void __attribute__((noreturn)) panic(const char *fmt, ...);

#endif