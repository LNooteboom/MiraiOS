#ifndef __PHLIBC_STDLIB_H
#define __PHLIBC_STDLIB_H

#if defined(__cplusplus)
extern "C" {
#endif

void abort(void);

void exit(int code);

void free(void *ptr);

void *malloc(long size);

int atexit(void (*function)(void));

int atoi(const char *nptr);

char *getenv(const char *name);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif