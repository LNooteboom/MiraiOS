#ifndef __PHLIBC_STDLIB_H__
#define __PHLIBC_STDLIB_H__

void abort(void);

void free(void *ptr);

void *malloc(long size);

int atexit(void (*function)(void));

int atoi(const char *nptr);

char *getenv(const char *name);

#endif