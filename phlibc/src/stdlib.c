#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <uapi/syscalls.h>
#include <sys/wait.h>

char *getenv(const char *name) {
	char *curEnv = *environ;
	int i = 1;
	int nameLen = strlen(name);
	while (curEnv) {
		if (!memcmp(name, curEnv, nameLen) && curEnv[nameLen] == '=') {
			return &curEnv[nameLen + 1];
		}
		curEnv = environ[i++];
	}
	return NULL;
}

pid_t fork(void) {
	pid_t ret = sysFork();
	if (ret < 0) {
		errno = ret;
		return -1;
	}
	return ret;
}

pid_t waitpid(pid_t pid, int *wstatus, int options) {
	pid_t ret = sysWaitPid(pid, wstatus, options);
	if (ret < 0) {
		errno = ret;
		return -1;
	}
	return ret;
}

int chdir(const char *path) {
	int ret = sysChDir(path);
	if (ret) {
		errno = ret;
		return -1;
	}
	return 0;
}