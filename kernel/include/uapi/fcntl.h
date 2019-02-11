#ifndef __PHLIBC_UAPI_FCNTL_H
#define __PHLIBC_UAPI_FCNTL_H

#define PIPE_BUF	4096

#define SYSOPEN_FLAG_CREATE		(1 << 0)
#define SYSOPEN_FLAG_READ		(1 << 1)
#define SYSOPEN_FLAG_WRITE		(1 << 2)
#define SYSOPEN_FLAG_CLOEXEC	(1 << 3)
#define SYSOPEN_FLAG_APPEND		(1 << 4)
#define SYSOPEN_FLAG_DIR		(1 << 5)
#define SYSOPEN_FLAG_EXCL		(1 << 6)
#define SYSOPEN_FLAG_TRUNC		(1 << 7)
#define SYSOPEN_PERM_SHIFT		8
#define SYSOPEN_PERM_MASK		0xFFF

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define SYSACCESS_OK			(1 << 0)
#define SYSACCESS_X				(1 << 1)
#define SYSACCESS_W				(1 << 2)
#define SYSACCESS_R				(1 << 3)

#define AT_FDCWD				-1
#define AT_REMOVEDIR			(1 << 0)

#define RENAME_NOREPLACE		(1 << 0)

#endif