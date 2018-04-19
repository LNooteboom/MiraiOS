#ifndef __PHLIBC_SYS_TYPES_H
#define __PHLIBC_SYS_TYPES_H

#include <phlibc/intsizes.h>

#ifndef __PHLIBC_DEF_PID_T
#define __PHLIBC_DEF_PID_T
typedef __PHLIBC_TYPE_PID_T pid_t;
#endif

#ifndef __PHLIBC_DEF_SIZE_T
#define __PHLIBC_DEF_SIZE_T
typedef __PHLIBC_TYPE_SIZE_T size_t;
#endif

#ifndef __PHLIBC_DEF_SSIZE_T
#define __PHLIBC_DEF_SSIZE_T
typedef __PHLIBC_TYPE_SSIZE_T ssize_t;
#endif

#ifndef __PHLIBC_DEF_OFF_T
#define __PHLIBC_DEF_OFF_T
typedef __PHLIBC_TYPE_OFF_T off_t;
#endif

#ifndef __PHLIBC_DEF_UID_T
#define __PHLIBC_DEF_UID_T
typedef __PHLIBC_TYPE_OFF_T uid_t;
#endif

#ifndef __PHLIBC_DEF_GID_T
#define __PHLIBC_DEF_GID_T
typedef __PHLIBC_TYPE_OFF_T gid_t;
#endif

#endif