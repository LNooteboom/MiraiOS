#ifndef __PHLIBC_STDLIB_H
#define __PHLIBC_STDLIB_H

#include <phlibc/intsizes.h>

#ifndef __PHLIBC_DEF_SIZE_T
#define __PHLIBC_DEF_SIZE_T
typedef __PHLIBC_TYPE_SIZE_T size_t;
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#define RAND_MAX	0xFFFFFFFF
#define EXIT_FAILURE	(-1)
#define EXIT_SUCCESS	0

static inline int abs(int i) {
	return (i < 0)? -i : i;
}

long random(void);
void srandom(unsigned int seed);
int rand(void);
void srand(unsigned int seed);

void exit(int code) __attribute__((noreturn));
void _exit(int code) __attribute__((noreturn));

//void abort(void);
__attribute__((noreturn))  static inline void abort(void) {
	_exit(-1);
}

void free(void *ptr);
void *malloc(size_t size) __attribute__((malloc, assume_aligned(16)));
void *calloc(size_t nmemb, size_t size) __attribute__((malloc, assume_aligned(16)));
void *realloc(void *ptr, size_t size);

int atexit(void (*function)(void));

int system(const char *command);

int atoi(const char *nptr);

char *getenv(const char *name);

double strtod(const char *s, char **endp);

int mkstemp(char *template);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif