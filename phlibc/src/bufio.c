#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <uapi/syscalls.h>
#include <uapi/fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define STREAM_MODEMASK	3
#define STREAM_FREEBUF	(1 << 2)

extern FILE _PHStdout;
extern FILE _PHStderr;

char _PHStdinBuf[BUFSIZ];
FILE _PHStdin = {
	.fd = 0,
	.flags = _IOLBF,
	.buf = _PHStdinBuf,
	.writeEnd = _PHStdinBuf,
	.readEnd = _PHStdinBuf,
	.bufEnd = _PHStdinBuf + BUFSIZ,
	.next = &_PHStdout
};

char _PHStdoutBuf[BUFSIZ];
FILE _PHStdout = {
	.fd = 1,
	.flags = _IOLBF,
	.buf = _PHStdoutBuf,
	.writeEnd = _PHStdoutBuf,
	.readEnd = _PHStdoutBuf,
	.bufEnd = _PHStdoutBuf + BUFSIZ,
	.next = &_PHStderr,
	.prev = &_PHStdin
};

char _PHStderrBuf[BUFSIZ];
FILE _PHStderr = {
	.fd = 2,
	.flags = _IOLBF,
	.buf = _PHStderrBuf,
	.writeEnd = _PHStderrBuf,
	.readEnd = _PHStderrBuf,
	.bufEnd = _PHStderrBuf + BUFSIZ,
	.prev = &_PHStdout
};

FILE *stdin = &_PHStdin;
FILE *stdout = &_PHStdout;
FILE *stderr = &_PHStderr;

FILE *_PHFirstFile = &_PHStdin;
FILE *_PHLastFile = &_PHStderr;

FILE *fopen(const char *filename, const char *mode) {
	FILE *f = NULL;
	if (!mode || !*mode) {
		errno = -EINVAL;
		goto ret;
	}
	unsigned int flags = 0;
	switch (mode[0]) {
		case 'r':
			flags = SYSOPEN_FLAG_READ;
			if (mode[1]) {
				if (mode[1] == '+') {
					flags |= SYSOPEN_FLAG_WRITE;
				} else {
					errno = -EINVAL;
					goto ret;
				}
			}
			break;
		case 'w':
			flags = SYSOPEN_FLAG_WRITE | SYSOPEN_FLAG_TRUNC;
			if (mode[1]) {
				if (mode[1] == '+') {
					flags |= SYSOPEN_FLAG_READ;
				} else {
					errno = -EINVAL;
					goto ret;
				}
			}
			break;
		case 'a':
			flags = SYSOPEN_FLAG_WRITE | SYSOPEN_FLAG_APPEND;
			if (mode[1]) {
				if (mode[1] == '+') {
					flags |= SYSOPEN_FLAG_READ;
				} else {
					errno = -EINVAL;
					goto ret;
				}
			}
			break;
		default:
			errno = -EINVAL;
			goto ret;
	}

	f = malloc(sizeof(*f));
	if (!f) {
		goto ret;
	}
	f->buf = malloc(BUFSIZ);
	if (!f->buf) {
		goto freeFile;
	}
	f->bufEnd = f->buf + BUFSIZ;
	f->writeEnd = f->buf;
	f->readEnd = f->buf;
	f->flags = _IOFBF | STREAM_FREEBUF;

	f->prev = _PHLastFile;
	_PHLastFile->next = f;
	_PHLastFile = f;

	int error = sysOpen(AT_FDCWD, filename, flags);
	if (error) {
		errno = error;
		goto freeBuf;
	}
	f->fd = error;

	freeBuf:
	free(f->buf);
	freeFile:
	free(f);
	f = NULL;
	ret:
	return f;
}

int fflush(FILE *stream) {
	size_t writeSize = stream->writeEnd - stream->buf;
	int error = 0;
	if (!writeSize) {
		return 0;
	}
	error = sysWrite(stream->fd, stream->buf, writeSize);
	stream->writeEnd = stream->buf;
	stream->readEnd = stream->buf;
	if (error < 0) {
		errno = error;
		return -1;
	}
	return 0;
}

int fclose(FILE *stream) {
	int error = fflush(stream);
	if (error) return error;

	if (stream->prev) {
		stream->prev->next = stream->next;
	} else {
		_PHFirstFile = stream->next;
	}
	if (stream->next) {
		stream->next->prev = stream->prev;
	} else {
		_PHLastFile = stream->prev;
	}

	error = sysClose(stream->fd);
	if (stream->flags & STREAM_FREEBUF) {
		free(stream->buf);
	}
	if (stream != stdin && stream != stdout && stream != stderr) {
		free(stream);
	}

	if (error < 0) {
		errno = error;
		return -1;
	}
	return 0;
}

void _PHCloseAll(void) {
	FILE *f = _PHFirstFile;
	while (f) {
		FILE *next = f->next;
		fclose(f);
		f = next;
	}
}

int setvbuf(FILE *stream, char *buffer, int mode, size_t size) {
	if (fflush(stream)) return -1;
	if (stream->flags & STREAM_FREEBUF) {
		free(stream->buf);
	}

	if (!buffer) {
		buffer = malloc(size);
		if (!buffer) {
			return -1;
		}
		stream->flags |= STREAM_FREEBUF;
	} else {
		stream->flags &= ~STREAM_FREEBUF;
	}
	stream->buf = buffer;
	stream->writeEnd = stream->readEnd = stream->buf;
	stream->bufEnd = stream->buf + size;

	stream->flags &= ~STREAM_MODEMASK;
	stream->flags |= mode & STREAM_MODEMASK;
	return 0;
}

void setbuf(FILE *stream, char *buffer) {
	setvbuf(stream, buffer, stream->flags, BUFSIZ);
}

static size_t fwriteFBF(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	size_t totalSize = size * nmemb;
	size_t writeSize = stream->bufEnd - stream->writeEnd;
	const char *src = (const char *)ptr;
	if (totalSize > writeSize) {
		memcpy(stream->writeEnd, src, writeSize);
		//stream->writeEnd = stream->bufEnd;
		//stream->readEnd = stream->bufEnd;
		src += writeSize;
		totalSize -= writeSize;

		if (fflush(stream)) return 0;

		if (totalSize > (size_t)(stream->bufEnd - stream->buf)) {
			if (write(stream->fd, ptr, totalSize) < 0) return 0;
			return nmemb;
		}
	}
	memcpy(stream->writeEnd, src, totalSize);
	stream->writeEnd += totalSize;
	return nmemb;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
	if ((stream->flags & STREAM_MODEMASK) == _IOLBF) {
		size_t totalSize = size * nmemb;
		const char *str = ptr;
		while (totalSize) {
			const char *end = memchr(str, '\n', totalSize);
			if (end) {
				end++;
			} else {
				end = str + totalSize;
			}
			size_t diff = end - str;

			if (!fwriteFBF(str, 1, diff, stream)) return 0;
			if (fflush(stream) < 0) return 0;

			totalSize -= diff;
			str += diff;
		}
	} else if ((stream->flags & STREAM_MODEMASK) == _IOFBF) {
		return fwriteFBF(ptr, size, nmemb, stream);
	} else {
		//_IONBF
		if (fflush(stream) < 0) return 0;
		if (write(stream->fd, ptr, size * nmemb) < 0) return 0;
	}
	return nmemb;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	//TODO? Kernel VFS already does caching
	ssize_t rd = read(stream->fd, ptr, size * nmemb);
	if (rd <= 0) {
		return 0;
	}
	return rd / size;
}