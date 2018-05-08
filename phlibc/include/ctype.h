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
static inline int iscntrl(int c) {
	return (c < 0x20 && c);
}
static inline int isgraph(int c) {
	return (c > 0x20 && c < 0x7F);
}
static inline int islower(int c) {
	return (c >= 'a' && c <= 'z');
}
static inline int isupper(int c) {
	return (c >= 'A' && c <= 'Z');
}
static inline int ispunct(int c) {
	return (isgraph(c) && !isalnum(c));
}
static inline int isprint(int c) {
	return (isgraph(c) || isspace(c));
}


static inline int toupper(int c) {
	if (c >= 'a' && c <= 'z') {
		c += ('a' - 'A');
	}
	return c;
}
static inline int tolower(int c) {
	if (c >= 'A' && c <= 'Z') {
		c += ('A' - 'a');
	}
	return c;
}

#endif