#ifndef __PHLIBC_STRING_H__
#define __PHLIBC_STRING_H__

void *memcpy(void *dest, const void *src, size_t n);

void *memset(void *str, int c, size_t n);

size_t strlen(const char *str);

char *strcpy(char *dest, const char *src);

#endif