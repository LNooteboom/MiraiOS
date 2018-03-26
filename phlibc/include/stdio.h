#ifndef __PHLIBC_STDIO_H
#define __PHLIBC_STDIO_H

#include <stdarg.h>
#include <intsizes.h>

#define SEEK_SET 0

#ifndef EOF
#define EOF -1
#endif

#ifndef __PHLIBC_DEF_SIZE_T
#define __PHLIBC_DEF_SIZE_T
typedef __PHLIBC_TYPE_SIZE_T size_t;
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct {
	int fd;
} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int fputs(const char *restrict s, FILE *restrict stream);

static inline int fputc(int c, FILE *stream) {
	char buf[2];
	buf[0] = c;
	buf[1] = 0;
	return fputs(buf, stream);
}

static inline int puts(const char *s) {
	int error = fputs(s, stdout);
	if (error) return error;
	fputc('\n', stdout);
}

static inline int putc(int c, FILE *stream) {
	return fputc(c, stream);
}

char *fgets(char *s, size_t size, FILE *stream);




int vfprintf(FILE *stream, const char *format, va_list arg);

static inline int fprintf(FILE *stream, const char *format, ...) {
	va_list args;
	va_start(args, format);

	int error = vfprintf(stream, format, args);

	va_end(args);
	return error;
}

static inline int printf(const char *format, ...) {
	va_list args;
	va_start(args, format);

	int error = vfprintf(stdout, format, args);

	va_end(args);
	return error;
}


int fclose(FILE *stream);

FILE *fopen(const char *filename, const char *mode);


int fflush(FILE *stream);

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

int fseek(FILE *stream, long int offset, int whence);

long ftell(FILE *stream);

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);


void setbuf(FILE *stream, char *buffer);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif