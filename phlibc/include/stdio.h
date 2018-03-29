#ifndef __PHLIBC_STDIO_H
#define __PHLIBC_STDIO_H

#include <stdarg.h>
#include <phlibc/intsizes.h>

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
int fputc(int c, FILE *stream);
int puts(const char *s);
int putc(int c, FILE *stream);

char *fgets(char *s, size_t size, FILE *stream);


int vfprintf(FILE *stream, const char *format, va_list arg);
int fprintf(FILE *stream, const char *format, ...);
int printf(const char *format, ...);


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