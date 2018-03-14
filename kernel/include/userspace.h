#ifndef INCLUDE_USERSPACE_H
#define INCLUDE_USERSPACE_H

#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <stdbool.h>

/*
Validate whether a pointer points to userspace memory, doesn't check page mapping
*/
static inline int validateUserPointer(const void *ptr, size_t size) {
	uintptr_t uptr = (uintptr_t)ptr;
	if (uptr & 0xFFFF8000UL << 32 || (0xFFFF8000UL << 32) - uptr < size) {
		return -EINVAL;
	}
	return 0;
}

/*
Validate whether a string is located is userspace memory
*/
static inline int validateUserString(const char *str) {
	while (true) {
		if ((uintptr_t)str & 0xFFFF8000UL << 32) {
			return -EINVAL;
		}
		if (!*str) {
			return 0;
		}
		str++;
	}
}

#endif