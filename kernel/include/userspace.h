#ifndef INCLUDE_USERSPACE_H
#define INCLUDE_USERSPACE_H

#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <stdbool.h>
#include <mm/memset.h>

struct UserAccBuf {
	uint64_t r15; //TODO make this arch independent
	uint64_t r14;
	uint64_t r13;
	uint64_t r12;
	uint64_t rbp;
	uint64_t rbx;
	uint64_t rip;
};
int _userAcc(struct UserAccBuf *buf);

#define USER_ACC_TRY(b, e)		if ((e = _userAcc(&b)) == 0)
#define USER_ACC_END()			do {			\
	thread_t curThread = getCurrentThread();	\
	if (curThread) {							\
		curThread->userAccBuf = NULL;			\
	}} while (0)
#define USER_ACC_CATCH			else

static inline int userMemcpy(void *dst, void *src, size_t n) {
	int error;
	struct UserAccBuf b;
	USER_ACC_TRY(b, error) {
		memcpy(dst, src, n);
		USER_ACC_END();
	}
	return error;
}

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

static inline int validateUserStringL(const char *str) {
	int len = 0;
	while (true) {
		if ((uintptr_t)str & 0xFFFF8000UL << 32) {
			return -EINVAL;
		}
		if (!*str) {
			return len;
		}
		str++;
		len++;
	}
}

#endif