#include <unistd.h>
#include <uapi/syscalls.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <uapi/fcntl.h>
#include <stdarg.h>

#include <stdio.h>

int execve(const char *filename, char *const argv[], char *const envp[]) {
	int error = sysExec(filename, argv, envp);
	//This should only be executed on error
	errno = error;
	return -1;
}

int execvpe(const char *file, char *const argv[], char *const envp[]) {
	if (strchr(file, '/')) {
		return execve(file, argv, envp);
	}
	//look in PATH
	char *path = getenv("PATH");
	if (!path) {
		errno = -ENOENT;
		return -1;
	}
	
	int nameLen = strlen(file);
	char buf[512];
	
	const char *dir = path;
	while (true) {
		const char *dirEnd = strchrnul(dir, ':');
		int dirLen = dirEnd - dir;

		memcpy(buf, dir, dirLen);
		buf[dirLen] = '/';
		memcpy(&buf[dirLen + 1], file, nameLen);
		buf[dirLen + 1 + nameLen] = 0;

		if (!sysAccess(buf, SYSACCESS_X)) {
			return execve(buf, argv, envp);
		}
	
		if (!*dirEnd) {
			break;
		}
		dir = dirEnd + 1;
	}
	errno = -ENOENT;
	return -1;
}

int execlp(const char *file, const char *arg, ...) {
	va_list args;
	va_start(args, arg);
	unsigned int argc = 1;
	for (; va_arg(args, const char *); argc++) {
		if (argc >= 512 / sizeof(const char *)) {
			errno = -E2BIG;
			return -1;
		}
	}
	va_end(args);

	char *argv[argc];
	argv[0] = (char *)arg;
	va_start(args, arg);
	for (unsigned int i = 1; i < argc; i++) {
		argv[i] = (char *)va_arg(args, const char *);
	}
	va_end(args);
	argv[argc] = NULL;

	return execvp(file, argv);
}