#ifndef INCLUDE_HEAP_H
#define INCLUDE_HEAP_H

#include <stddef.h>

void *kmalloc(size_t size);

void kfree(void *mem);

void *krealloc(void *addr, size_t newSize);

#endif
