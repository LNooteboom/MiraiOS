#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <uapi/syscalls.h>
#include <sys/wait.h>
#include <locale.h>
#include <signal.h>

static char *errorTbl[] = {
	[0] = "",
	[EINVAL] = "Invalid argument",
	[EBUSY] = "Device or resource busy",
	[EIO] = "I/O error",
	[ENOENT] = "File or directory not found",
	[EISDIR] = "Is a directory",
	[ENOTDIR] = "Is not a directory",
	[ENOSYS] = "Not supported",
	[EROFS] = "Read-only file system",
	[EBADF] = "Bad file descriptor",
	[EFAULT] = "Fault occured",
	[ECHILD] = "Child does not exist",
	[EEXIST] = "File or directory exists",
	[ENOMEM] = "Out of memory",
	[E2BIG] = "Argument list too big",
	[EACCES] = "Access denied",
	[EPERM] = "Permission denied",
	[ESRCH] = "No such process"
};

static struct lconv loc;

char *strerror(int err) {
	return errorTbl[err];
}

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
		errno = -ret;
		return -1;
	}
	return ret;
}

pid_t waitpid(pid_t pid, int *wstatus, int options) {
	siginfo_t stat;
	pid_t ret = sysWaitPid(pid, &stat, options);
	if (wstatus) {
		*wstatus = stat.si_status;
	}
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

int chdir(const char *path) {
	int ret = sysChDir(path);
	if (ret) {
		errno = -ret;
		return -1;
	}
	return 0;
}

pid_t getpid(void) {
	return sysGetId(0, SYSGETID_PID); //can never fail
}
pid_t getppid(void) {
	return sysGetId(0, SYSGETID_PPID);
}
int setpgid(pid_t pid, pid_t pgid) {
	int error = sysSetpgid(pid, pgid);
	if (error) {
		errno = -error;
		return -1;
	}
	return 0;
}
pid_t getpgid(pid_t pid) {
	pid_t ret = sysGetId(pid, SYSGETID_PGID);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}
pid_t setsid(void) {
	pid_t ret = sysSetsid();
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}
pid_t getsid(pid_t pid) {
	pid_t ret = sysGetId(pid, SYSGETID_SID);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

char *setlocale(int category, const char *locale) {
	errno = ENOSYS;
	return NULL;
}

struct lconv *localeconv(void) {
	return &loc;
}