#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <uapi/syscalls.h>
#include <stdarg.h>
#include <uapi/getDent.h>
#include <uapi/fcntl.h>
#include <uapi/termios.h>

static int tmpNr;

ssize_t write(int fd, const void *buf, size_t size) {
	ssize_t ret = sysWrite(fd, buf, size);
	if (ret) {
		errno = -ret;
		return -1;
	}
	return size;
}

ssize_t read(int fd, void *buf, size_t size) {
	ssize_t ret = sysRead(fd, buf, size);
	if (ret < 0) {
		errno = -ret;
		return -1;
	}
	return ret;
}

int close(int fd) {
	int error = sysClose(fd);
	if (error) {
		errno = -error;
		return -1;
	}
	return 0;
}

int ioctl(int fd, unsigned long request, ...) {
	va_list args;
	va_start(args, request);
	unsigned long arg = va_arg(args, unsigned long);
	va_end(args);
	int error = sysIoctl(fd, request, arg);
	if (error < 0) {
		errno = -error;
		return -1;
	}
	return error;
}

int fputs(const char *restrict s, FILE *restrict stream) {
	size_t len = strlen(s);
	/*int error = sysWrite(stream->fd, s, len);
	if (error) {
		errno = -error;
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
		errno = -read;
		return NULL;
	}*/
	size_t read = fread(s, 1, size - 1, stream);
	if (!read) return NULL;
	s[read] = 0;
	return s;
}

int fgetc(FILE *stream) {
	int ret = 0;
	if (!fread(&ret, 1, 1, stream)) {
		ret = EOF;
	}
	return ret;
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

int putchar(int c) {
	return putc(c, stdout);
}

int ungetc(int c, FILE *stream) {
	if (stream->cbuf != EOF) {
		return EOF;
	}
	stream->cbuf = c;
	return c;
}

FILE *tmpfile(void) {
	char namebuf[NAME_MAX];
	snprintf(namebuf, NAME_MAX, "/tmp/%d.%d", getpid(), ++tmpNr);
	return fopen(namebuf, "w+");
}

int mkstemp(char *template) {
	int tempLen = strlen(template);
	int xPos = tempLen - 6;
	if (memcmp(&template[xPos], "XXXXXX", 6)) {
		errno = EINVAL;
		return -1;
	}

	snprintf(&template[xPos], 7, "%.*X%.*X", 4, getpid(), 2, ++tmpNr);
	
	char nameBuf[tempLen + 6];
	snprintf(nameBuf, tempLen + 6, "/tmp/%s", template);

	int fd = sysOpen(AT_FDCWD, nameBuf, SYSOPEN_FLAG_CREATE | SYSOPEN_FLAG_TRUNC | SYSOPEN_FLAG_WRITE);
	if (fd < 0) {
		errno = -fd;
		return -1;
	}
	return fd;
}

int fseeko(FILE *stream, off_t offset, int whence) {
	int error = sysSeek(stream->fd, offset, whence);
	if (error) {
		errno = -error;
		return -1;
	}
	switch (whence) {
		case SEEK_CUR:
			stream->seekOffset += offset;
			break;
		case SEEK_SET:
			stream->seekOffset = offset;
			break;
		case SEEK_END:
			//TODO
			break;
	}
	return 0;
}

off_t ftello(FILE *stream) {
	return stream->seekOffset + (stream->writeEnd - stream->readEnd);
}

int remove(const char *path) {
	int error = sysUnlink(AT_FDCWD, path, AT_REMOVEDIR);
	if (error) {
		errno = -error;
		return -1;
	}
	return 0;
}

int rename(const char *old, const char *newf) {
	int error = sysRename(AT_FDCWD, old, AT_FDCWD, newf, 0);
	if (error) {
		errno = -error;
		return -1;
	}
	return 0;
}

int isatty(int fd) {
	pid_t arg;
	int error = ioctl(fd, TIOCGPGRP, &arg);
	if (error) {
		errno = -error;
		return 0;
	}
	return 1;
}