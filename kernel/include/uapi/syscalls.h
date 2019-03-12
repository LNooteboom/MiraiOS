#ifndef __PHLIBC_SYSCALLS_H
#define __PHLIBC_SYSCALLS_H

#include <stdint.h>
#include <stddef.h>
#include "types.h"

#define SYSGETID_PID	0
#define SYSGETID_PPID	1
#define SYSGETID_PGID	2
#define SYSGETID_SID	3

#define SYSGETID_UID	4
#define SYSGETID_EUID	5
#define SYSGETID_GID	6
#define SYSGETID_EGID	7

#define SYSSETID_UID	4
#define SYSSETID_EUID	5
#define SYSSETID_GID	6
#define SYSSETID_EGID	7

struct GetDent;
struct sigaction;
struct stat;

//SYSCALLS
/*00*/ int sysRead(int fd, void *buffer, size_t size);
/*01*/ int sysWrite(int fd, const void *buffer, size_t size);
/*02*/ int sysIoctl(int fd, unsigned long request, unsigned long arg);
/*03*/ int sysOpen(int dirfd, const char *fileName, unsigned int flags);
/*04*/ int sysClose(int fd);
/*05*/ int sysGetDent(int fd, struct GetDent *buf);
/*06*/ int sysExec(const char *fileName, char *const argv[], char *const envp[]);
/*07*/ pid_t sysFork(void);
/*08*/ void sysExit(int exitValue);
/*09*/ pid_t sysWaitPid(pid_t filter, void *waitStatus, int options);
/*0A*/ int sysSleep(uint64_t seconds, uint32_t nanoSeconds);
/*0B*/ int sysArchPrctl(int which, void *addr);
/*0C*/ void* sysMmap(void *addr, size_t size, int flags, int fd, uint64_t offset);
/*0D*/ int sysMunmap(void *vaddr, size_t size);
/*0E*/ int sysPipe(int fd[2], int flags);
/*0F*/ int sysDup(int oldFD, int newFD, int flags);
/*10*/ int sysChDir(const char *path);
/*11*/ int sysAccess(int dirfd, const char *path, int mode);
/*12*/ pid_t sysSetsid(void);
/*13*/ int sysSetpgid(pid_t pid, pid_t pgid);
/*14*/ pid_t sysGetId(pid_t pid, int which);
/*15*/ int sysSetId(pid_t id, int which);
/*16*/ void sysSigRet(void);
/*17*/ int sysSigHandler(int sigNum, struct sigaction *action, struct sigaction *oldAction);
/*18*/ int sysSigprocmask(int how, const uint64_t *newset, uint64_t *oldset);
/*19*/ int sysSeek(int fd, int64_t offset, int whence);
/*1A*/ int sysUnlink(int dirfd, const char *path, int flags);
/*1B*/ int sysRename(int oldDirfd, const char *oldPath, int newDirfd, const char *newPath, int flags);
/*1C*/ int sysStat(const char *fileName, struct stat *statBuf);
/*1D*/ int sysLstat(const char *fileName, struct stat *statBuf);
/*1E*/ int sysFstatat(int dirfd, const char *fileName, struct stat *statBuf, int flags);
/*1F*/ int sysSetReuid(uid_t ruid, uid_t euid, int which);
//END

#endif