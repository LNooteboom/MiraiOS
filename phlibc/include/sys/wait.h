#ifndef __PHLIBC_SYS_WAIT_H
#define __PHLIBC_SYS_WAIT_H

#include <intsizes.h>

#ifndef __PHLIBC_DEF_PID_T
#define __PHLIBC_DEF_PID_T
typedef __PHLIBC_TYPE_PID_T pid_t;
#endif

#if defined(__cplusplus)
extern "C" {
#endif

pid_t waitpid(pid_t pid, int *wstatus, int options);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif