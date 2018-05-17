#ifndef INCLUDE_MM_MEMSET_H
#define INCLUDE_MM_MEMSET_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
Sets n bytes of data at str to c
*/
static inline void memset(volatile void *str, char c, size_t n) {
	if (n & 7) {
    	asm volatile ("rep stosb": "+D"(str), "+a"(c), "+c"(n) : : "memory");
	} else {
		n >>= 3;
		asm volatile ("rep stosq": "+D"(str), "+a"(c), "+c"(n) : : "memory");
	}
}

/*
Copies n bytes of data from src to dst
*/
static inline void memcpy(volatile void *dst, const void *src, size_t n) {
	if (n & 7) {
		asm volatile ("rep movsb": "+S"(src), "+D"(dst), "+c"(n) : : "memory");
	} else {
		n >>= 3;
		asm volatile ("rep movsq": "+S"(src), "+D"(dst), "+c"(n) : : "memory");
	}
}

/*
Compares n bytes of data.
Returns 0 if equal.
*/
static inline int memcmp(const void *a, const void *b, size_t n) {
	for (uintptr_t i = 0; i < n; i++) {
		if (((char*)a)[i] != ((char*)b)[i]) {
			return -1;
		}
	}
	return 0;
}

/*
Get the length of a string
*/
static inline size_t strlen(const char *str) {
	size_t result = 0;
	while (*str++) {
		result++;
	}
	return result;
}

/*
Find a char in a string
*/
static inline int findChar(const char *str, char c, size_t len, int pos) {
	for (unsigned int i = pos; i < len; i++) {
		if (str[i] == c) {
			return i;
		}
	}
	return -1;
}

static inline uintptr_t align(uintptr_t ptr, size_t align) {
	uintptr_t mod = ptr % align;
	if (mod) {
		ptr -= mod;
		ptr += align;
	}
	return ptr;
}

static inline uintptr_t alignLow(uintptr_t ptr, size_t align) {
	return ptr - ptr % align;
}

#endif