#ifndef INCLUDE_MEMSET_H
#define INCLUDE_MEMSET_H

#include <global.h>

static inline void memset(volatile void *str, char c, size_t n) {
    asm("rep stosb": :"D"(str), "a"(c), "c"(n));
}

#endif