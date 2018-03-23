#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void _PHAssert(const char *expr, const char *file, int line) {
	fprintf(stderr, "Assertion failed: %s in %s at line %d\n", expr, file, line);
	exit(-1);
}