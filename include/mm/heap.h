#ifndef INCLUDE_HEAP_H
#define INCLUDE_HEAP_H

#include <stddef.h>

void *vmalloc(size_t size);

void kfree(void *mem);

#endif
