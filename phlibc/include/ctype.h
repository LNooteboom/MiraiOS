#ifndef __PHLIBC_CTYPE_H
#define __PHLIBC_CTYPE_H

static inline int isdigit(int c) {
	return (c >= '0' && c <= '9');
}

static inline int isalpha(int c) {
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

static inline int isalnum(int c) {
	return (isdigit(c) || isalpha(c));
}

static inline int isxdigit(int c) {
	return (isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

static inline int isspace(int c) {
	return (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f');
}

#endif