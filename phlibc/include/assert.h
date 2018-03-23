#ifndef __PHLIBC_ASSERT_H
#define __PHLIBC_ASSERT_H

#ifdef NDEBUG
#define assert(exp)
#else
#define assert(exp) (void)((exp) || (_PHAssert(#exp, __FILE__, __LINE__),0))
#endif

#if defined(__cplusplus)
extern "C" {
#endif

void _PHAssert(const char *expr, const char *file, int line);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif