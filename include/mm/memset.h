#ifndef INCLUDE_MM_MEMSET_H
#define INCLUDE_MM_MEMSET_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
Sets n bytes of data at str to c
*/
static inline void memset(volatile void *str, char c, size_t n) {
    asm volatile ("rep stosb": "+D"(str), "+a"(c), "+c"(n) : : "memory");
}

/*
Copies n bytes of data from src to dst
*/
static inline void memcpy(volatile void *dst, const void *src, size_t n) {
	asm volatile ("rep movsb": "+S"(src), "+D"(dst), "+c"(n) : : "memory");
}

/*
Compares n bytes of data.
Returns true if equal.
*/
static inline bool memcmp(const void *a, const void *b, size_t n) {
	for (uintptr_t i = 0; i < n; i++) {
		if (((char*)a)[i] != ((char*)b)[i]) {
			return false;
		}
	}
	return true;
}

static inline size_t strlen(const char *str) {
	size_t result = 0;
	while (*str++) {
		result++;
	}
	return result;
}

#endif