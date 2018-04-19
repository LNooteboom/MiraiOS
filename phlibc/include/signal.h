#ifndef __PHLIBC_SIGNAL_H
#define __PHLIBC_SIGNAL_H

#include <phlibc/intsizes.h>

#ifndef __PHLIBC_DEF_PID_T
#define __PHLIBC_DEF_PID_T
typedef __PHLIBC_TYPE_PID_T pid_t;
#endif

#ifndef __PHLIBC_DEF_UID_T
#define __PHLIBC_DEF_UID_T
typedef __PHLIBC_TYPE_OFF_T uid_t;
#endif

typedef int sig_atomic_t;

#include <uapi/signal.h>

#define SIG_ERR	((void *)0)

#if defined(__cplusplus)
extern "C" {
#endif

void (*signal(int sig, void (*func)(int)))(int);
int sigaction(int sig, const struct sigaction *act, struct sigaction *oldact);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif