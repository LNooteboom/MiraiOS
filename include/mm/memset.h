#ifndef INCLUDE_MEMSET_H
#define INCLUDE_MEMSET_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

static inline void memset(volatile void *str, char c, size_t n) {
    asm("rep stosb": :"D"(str), "a"(c), "c"(n));
}

static inline void memcpy(volatile void *dst, volatile void *src, size_t n) {
	asm("rep movsb": :"S"(src), "D"(dst), "c"(n));
}

static inline bool memcmp(void *a, void *b, size_t n) {
	for (uintptr_t i = 0; i < n; i++) {
		if (((char*)a)[i] != ((char*)b)[i]) {
			return false;
		}
	}
	return true;
}

#endif