#ifndef __PHLIBC_SYS_MMAN_H
#define __PHLIBC_SYS_MMAN_H

#include <phlibc/intsizes.h>
#include <uapi/mmap.h>

#define MAP_FAIL	((void *)~0)

#define PROT_NONE	0
#define PROT_READ	0
#define PROT_WRITE	MMAP_FLAG_WRITE
#define PROT_EXEC	MMAP_FLAG_EXEC

#define MAP_SHARED	MMAP_FLAG_SHARED
#define MAP_PRIVATE	0 //default
#define MAP_FIXED	MMAP_FLAG_FIXED
#define MAP_ANON	MMAP_FLAG_ANON
#define MAP_ANONYMOUS	MAP_ANON

#ifndef __PHLIBC_DEF_SIZE_T
#define __PHLIBC_DEF_SIZE_T
typedef __PHLIBC_TYPE_SIZE_T size_t;
#endif

#ifndef __PHLIBC_DEF_OFF_T
#define __PHLIBC_DEF_OFF_T
typedef __PHLIBC_TYPE_OFF_T off_t;
#endif

#if defined(__cplusplus)
extern "C" {
#endif

void *mmap(void *addr, size_t size, int prot, int flags, int fd, off_t offset);

int munmap(void *addr, size_t size);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif