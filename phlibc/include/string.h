#ifndef __PHLIBC_STRING_H
#define __PHLIBC_STRING_H

#include <phlibc/intsizes.h>

#ifndef __PHLIBC_DEF_SIZE_T
#define __PHLIBC__DEF_SIZE_T
typedef __PHLIBC_TYPE_SIZE_T size_t;
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#if defined(__cplusplus)
extern "C" {
#endif

void *memcpy(void *dest, const void *src, size_t n);

void *memset(void *str, int c, size_t n);

int memcmp(const void *a, const void *b, size_t n);

void *memchr(const void *s, int c, size_t n);

size_t strlen(const char *str);

char *strcpy(char *dest, const char *src);

const char *strchr(const char *s, int c);

const char *strchrnul(const char *s, int c);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif