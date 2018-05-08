#ifndef __PHLIBC_UNISTD_H
#define __PHLIBC_UNISTD_H

#include <phlibc/intsizes.h>

#ifndef __PHLIBC_DEF_PID_T
#define __PHLIBC_DEF_PID_T
typedef __PHLIBC_TYPE_PID_T pid_t;
#endif

#ifndef __PHLIBC_DEF_SIZE_T
#define __PHLIBC_DEF_SIZE_T
typedef __PHLIBC_TYPE_SIZE_T size_t;
#endif

#ifndef __PHLIBC_DEF_SSIZE_T
#define __PHLIBC_DEF_SSIZE_T
typedef __PHLIBC_TYPE_SSIZE_T ssize_t;
#endif

#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2

#if defined(__cplusplus)
extern "C" {
#endif

extern char **environ;

int chdir(const char *path);

ssize_t write(int fd, const void *buf, size_t size);
ssize_t read(int fd, void *buf, size_t size);
int close(int fd);

int isatty(int fd);


pid_t fork(void);

pid_t getpid(void);
pid_t getppid(void);
int setpgid(pid_t pid, pid_t pgid);
pid_t getpgid(pid_t pid);
pid_t setsid(void);
pid_t getsid(pid_t pid);

/* * * Exec * * */

int execve(const char *filename, char *const argv[], char *const envp[]);

static inline int execv(const char *path, char *const argv[]) {
	return execve(path, argv, environ);
}

int execvpe(const char *file, char *const argv[], char *const envp[]);

static inline int execvp(const char *file, char *const argv[]) {
	return execvpe(file, argv, environ);
}

int execlp(const char *file, const char *arg, ...);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif