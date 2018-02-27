#ifndef __PHLIBC_UNISTD_H__
#define __PHLIBC_UNISTD_H__

typedef long pid_t;

pid_t fork(void);

int execv(const char *path, char *const argv[]);
int execve(const char *filename, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);

#endif