#include <syscall.h>

extern int sysRead();
extern int sysWrite();
extern int sysIoctl();
extern int sysOpen();
extern int sysClose();
extern int sysGetDent();
extern int sysExec();
extern int sysFork();
extern int sysExit();
extern int sysWaitPid();
extern int sysSleep();
extern int sysArchPrctl();

int (*syscallTable[256])() = {
	[0] = sysRead,
	[1] = sysWrite,
	[2] = sysIoctl,
	[3] = sysOpen,
	[4] = sysClose,
	[5] = sysGetDent,

	[6] = sysExec,
	[7] = sysFork,
	[8] = sysExit,
	[9] = sysWaitPid,
	[10] = sysSleep,
	[11] = sysArchPrctl
};