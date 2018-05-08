#ifndef __PHLIBC_STDIO_H
#define __PHLIBC_STDIO_H

#include <stdarg.h>
#include <phlibc/intsizes.h>

#define BUFSIZ		512

#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2

#define _IOFBF		0
#define _IOLBF		1
#define _IONBF		2

#ifndef EOF
#define EOF -1
#endif

#define getc fgetc
#define getc_unlocked fgetc

#define fseek fseeko
#define ftell ftello

#ifndef __PHLIBC_DEF_SIZE_T
#define __PHLIBC_DEF_SIZE_T
typedef __PHLIBC_TYPE_SIZE_T size_t;
#endif

#ifndef __PHLIBC_DEF_OFF_T
#define __PHLIBC_DEF_OFF_T
typedef __PHLIBC_TYPE_OFF_T off_t;
#endif

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct _PHFILE{
	int fd;
	int flags;
	int pid;

	int cbuf;

	off_t seekOffset;

	char *buf;
	char *bufEnd;

	char *writeEnd;
	char *readStart;
	char *readEnd;

	struct _PHFILE *next;
	struct _PHFILE *prev;
} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int fputs(const char *restrict s, FILE *restrict stream);
int fputc(int c, FILE *stream);
int puts(const char *s);
int putc(int c, FILE *stream);
int putchar(int c);

int ungetc(int c, FILE *stream);

char *fgets(char *s, size_t size, FILE *stream);
int fgetc(FILE *stream);


int vfprintf(FILE *stream, const char *format, va_list arg);
int fprintf(FILE *stream, const char *format, ...);
int printf(const char *format, ...);

int sprintf(char *buf, const char *format, ...);
int snprintf(char *buf, size_t size, const char *format, ...);
int vsnprintf(char *buf, size_t size, const char *format, va_list arg);

int fclose(FILE *stream);
FILE *fopen(const char *filename, const char *mode);
FILE *freopen(const char *restrict pathname, const char *restrict mode, FILE *restrict stream);

int fflush(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

int fseeko(FILE *stream, off_t offset, int whence);
off_t ftello(FILE *stream);


void setbuf(FILE *stream, char *buffer);
int setvbuf(FILE *stream, char *buffer, int mode, size_t size);

int feof(FILE *stream);
int ferror(FILE *stream);
void clearerr(FILE *stream);

FILE *tmpfile(void);

FILE *popen(const char *command, const char *mode);
int pclose(FILE *stream);

static inline void flockfile(FILE *stream) {
	(void) stream;
}
static inline void funlockfile(FILE *stream) {
	(void) stream;
}

int remove(const char *path);
int rename(const char *old, const char *newf);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif