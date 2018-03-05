#ifndef __PHLIBC_SYSCALLS_H
#define __PHLIBC_SYSCALLS_H

#include <stdint.h>
#include <stddef.h>

#define SYSOPEN_FLAG_CREATE		(1 << 0)
#define SYSOPEN_FLAG_READ		(1 << 1)
#define SYSOPEN_FLAG_WRITE		(1 << 2)
#define SYSOPEN_FLAG_CLOEXEC	(1 << 3)
#define SYSOPEN_FLAG_APPEND		(1 << 4)

struct GetDent {
	uint32_t inodeID;
	unsigned int type;
	char name[256];
};

typedef long pid_t;

//SYSCALLS
/*00*/ int sysRead(int fd, void *buffer, size_t size);
/*01*/ int sysWrite(int fd, void *buffer, size_t size);
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
/*0C*/ void* sysSbrk(int64_t new);
//END

#endif