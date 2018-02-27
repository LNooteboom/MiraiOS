#ifndef __PHLIBC_STDIO_H__
#define __PHLIBC_STDIO_H__

#include <stdarg.h>

#define SEEK_SET 0

typedef struct {
	long test;
} _FILE;

typedef _FILE * FILE;

extern FILE stderr;

int fflush(FILE *stream);

int fprintf(FILE stream, const char *str, ...);

int vfprintf(FILE *stream, const char *format, va_list arg);


int fclose(FILE *stream);

FILE *fopen(const char *filename, const char *mode);


size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

int fseek(FILE *stream, long int offset, int whence);

long ftell(FILE *stream);

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);


void setbuf(FILE *stream, char *buffer);

#endif