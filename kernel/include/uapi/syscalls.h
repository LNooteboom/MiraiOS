#ifndef __PHLIBC_SYSCALLS_H
#define __PHLIBC_SYSCALLS_H

#include <stdint.h>
#include <stddef.h>
#include "types.h"

struct GetDent;

//SYSCALLS
/*00*/ int sysRead(int fd, void *buffer, size_t size);
/*01*/ int sysWrite(int fd, const void *buffer, size_t size);
/*02*/ int sysIoctl(int fd, unsigned long request, ...);
/*03*/ int sysOpen(const char *fileName, unsigned int flags);
/*04*/ int sysClose(int fd);
/*05*/ int sysGetDent(int fd, struct GetDent *buf);
/*06*/ int sysExec(const char *fileName, char *const argv[], char *const envp[]);
/*07*/ int sysFork(void);
/*08*/ void sysExit(int exitValue);
/*09*/ pid_t sysWaitPid(pid_t filter, int *waitStatus, int options);
/*0A*/ int sysSleep(uint64_t seconds, uint32_t nanoSeconds);
/*0B*/ int sysArchPrctl(int which, void *addr);
/*0C*/ void* sysMmap(void *addr, size_t size, int flags, int fd, uint64_t offset);
/*0D*/ int sysMunmap(void *vaddr, size_t size);
/*0E*/ int sysPipe(int fd[2], int flags);
/*0F*/ int sysDup(int oldFD, int newFD, int flags);
/*10*/ int sysChDir(const char *path);
/*11*/ int sysAccess(const char *path, int mode);
//END

#endif