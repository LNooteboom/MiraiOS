#ifndef INCLUDE_HEAP_H
#define INCLUDE_HEAP_H

#include <global.h>

#define HEAPSIZE 0x80000

void *kmalloc(size_t size);

void *vmalloc(size_t size);

void kfree(void *mem);

#endif
