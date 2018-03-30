#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <uapi/syscalls.h>

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
	/*int error = sysWrite(stream->fd, s, len);
	if (error) {
		errno = error;
		return -1;
	}
	return 0;*/
	if (fwrite(s, 1, len, stream)) {
		return 0;
	}
	return -1;
}

char *fgets(char *s, size_t size, FILE *stream) {
	/*ssize_t read = sysRead(stream->fd, s, size - 1);
	if (read < 0) {
		errno = read;
		return NULL;
	}*/
	size_t read = fread(s, 1, size - 1, stream);
	if (!read) return NULL;
	s[read] = 0;
	return s;
}

int fputc(int c, FILE *stream) {
	char buf[2];
	buf[0] = c;
	buf[1] = 0;
	return fputs(buf, stream);
}

int puts(const char *s) {
	int error = fputs(s, stdout);
	if (error) return error;
	return fputc('\n', stdout);
}

int putc(int c, FILE *stream) {
	return fputc(c, stream);
}