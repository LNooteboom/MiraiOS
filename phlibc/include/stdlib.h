#ifndef __PHLIBC_STDLIB_H
#define __PHLIBC_STDLIB_H

#include <phlibc/intsizes.h>

#ifndef __PHLIBC_DEF_SIZE_T
#define __PHLIBC_DEF_SIZE_T
typedef __PHLIBC_TYPE_SIZE_T size_t;
#endif

#if defined(__cplusplus)
extern "C" {
#endif

static inline int abs(int i) {
	return (i < 0)? -i : i;
}

void abort(void);

void exit(int code);

void free(void *ptr);
void *malloc(size_t size) __attribute__((malloc, assume_aligned(16)));
void *calloc(size_t nmemb, size_t size) __attribute__((malloc, assume_aligned(16)));

int atexit(void (*function)(void));

int atoi(const char *nptr);

char *getenv(const char *name);

double strtod(const char *s, char **endp);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif