#ifndef __PHLIBC_UAPI_TYPES_H
#define __PHLIBC_UAPI_TYPES_H

#define NGROUPS_MAX	32

#ifndef __PHLIBC_DEF_PID_T
#define __PHLIBC_DEF_PID_T
typedef int pid_t;
#endif

#ifndef __PHLIBC_DEF_UID_T
#define __PHLIBC_DEF_UID_T
typedef int uid_t;
#endif

#ifndef __PHLIBC_DEF_GID_T
#define __PHLIBC_DEF_GID_T
typedef int gid_t;
#endif

#endif