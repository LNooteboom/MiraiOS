#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <uapi/syscalls.h>

FILE _PHStdin = {
	.fd = 0
};
FILE _PHStdout = {
	.fd = 1
};
FILE _PHStderr = {
	.fd = 2
};
FILE *stdin = &_PHStdin;
FILE *stdout = &_PHStdout;
FILE *stderr = &_PHStderr;

ssize_t write(int fd, const void *buf, size_t size) {
	ssize_t ret = sysWrite(fd, buf, size);
	if (ret) {
		errno = ret;
		return -1;
	}
	return size;
}

ssize_t read(int fd, void *buf, size_t size) {
	ssize_t ret = sysRead(fd, buf, size);
	if (ret < 0) {
		errno = ret;
		return -1;
	}
	return ret;
}

int fputs(const char *restrict s, FILE *restrict stream) {
	size_t len = strlen(s);
	int error = sysWrite(stream->fd, s, len);
	if (error) {
		errno = error;
		return -1;
	}
	return 0;
}

char *fgets(char *s, size_t size, FILE *stream) {
	ssize_t read = sysRead(stream->fd, s, size - 1);
	if (read < 0) {
		errno = read;
		return NULL;
	}
	s[read] = 0;
	return s;
}