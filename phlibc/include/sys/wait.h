#ifndef __PHLIBC_SYS_WAIT_H
#define __PHLIBC_SYS_WAIT_H

#include <phlibc/intsizes.h>

#ifndef __PHLIBC_DEF_PID_T
#define __PHLIBC_DEF_PID_T
typedef __PHLIBC_TYPE_PID_T pid_t;
#endif

#define WIFEXITED(status)	((status & 0x00F00) == 0x00100)
#define WEXITSTATUS(status)	(status & 0x000FF)
#define WIFSIGNALED(status)	((status & 0x00F00) == 0x00300)
#define WTERMSIG(status)	((status >> 12) & 0xFF)

#if defined(__cplusplus)
extern "C" {
#endif

pid_t waitpid(pid_t pid, int *wstatus, int options);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif