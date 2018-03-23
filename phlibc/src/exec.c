#include <unistd.h>
#include <uapi/syscalls.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int execve(const char *filename, char *const argv[], char *const envp[]) {
	int error = sysExec(filename, argv, envp);
	//This should only be executed on error
	errno = error;
	return -1;
}
