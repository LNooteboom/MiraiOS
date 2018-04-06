#ifndef INCLUDE_HEAP_H
#define INCLUDE_HEAP_H

#include <stddef.h>

/*
Dynamically allocates an area of readable and writable memory.
Returns NULL on fail.
*/
void *kmalloc(size_t size);

void *kzalloc(size_t size);

/*
Frees memory allocated by kmalloc or krealloc.
*/
void kfree(void *mem);

/*
Changes the size of an already allocated memory area.
If addr == NULL then a new memory area is allocated.
Returns pointer to new memory or NULL on fail.
*/
void *krealloc(void *addr, size_t newSize);

#endif
