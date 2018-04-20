#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>

#define ADD_TO_BUF(buf, bufLen, used, c) 	\
	do {									\
		if (used == bufLen) {				\
			return -EINVAL;					\
		}									\
		buf[used++] = c;					\
	} while (0)

static bool isFlag(char c) {
	return (c == '-' || c == '+' || c == ' ' || c == '#' || c == '0');
}
static bool flagPresent(const char *flags, int nFlags, char flag) {
	for (int i = 0; i < nFlags; i++) {
		if (flags[i] == flag) {
			return true;
		}
	}
	return false;
}

static unsigned long truncateInt(unsigned long in, bool sign, char varLength) {
	switch (varLength) {
		case 0:
			in &= 0xFFFFFFFFUL;
			if (sign && in & 0x80000000) {
				in |= 0xFFFFFFFF00000000;
			}
			break;
		case 'h':
			in &= 0xFFFF;
			if (sign && in & 0x8000) {
				in |= 0xFFFFFFFFFFFF0000;
			}
			break;
	}
	return in;
}

static int printDec(char *buf, int bufLen, unsigned long in, bool sign,
		const char *flags, int nFlags, int width, int precision, char varLength) {
	int ret = 0;
	in = truncateInt(in, sign, varLength);

	if (sign && in & (1UL << 63)) {
		ADD_TO_BUF(buf, bufLen, ret, '-');
		in = (unsigned long)(-(long)in);
	} else if (flagPresent(flags, nFlags, '+')) {
		ADD_TO_BUF(buf, bufLen, ret, '+');
	} else if (flagPresent(flags, nFlags, ' ')) {
		ADD_TO_BUF(buf, bufLen, ret, ' ');
	}

	char temp[100];
	int nLen = 0;
	if (!in) {
		temp[0] = '0';
		nLen++;
	} else while (in != 0) {
		char c = (in % 10) + '0';
		in /= 10;
		ADD_TO_BUF(temp, 100, nLen, c);
	}
	
	while (precision - nLen > 0) {
		ADD_TO_BUF(temp, 100, nLen, '0');
	}
	char pad = (flagPresent(flags, nFlags, '0'))? '0' : ' ';
	bool lJustify = flagPresent(flags, nFlags, '-');
	if (lJustify) {
		while (width - nLen > 0) {
			ADD_TO_BUF(temp, 100, nLen, pad);
		}
	}

	//now copy temp to buf in reverse
	for (int i = nLen - 1; i >= 0; i--) {
		ADD_TO_BUF(buf, bufLen, ret, temp[i]);
	}
	if (!lJustify) {
		while (width - nLen > 0) {
			ADD_TO_BUF(buf, bufLen, ret, pad);
		}
	}
	return ret;
}

static int printHex(char *buf, int bufLen, unsigned long in, bool caps,
		const char *flags, int nFlags, int width, int precision, char varLength) {
	int ret = 0;
	in = truncateInt(in, false, varLength);

	if (flagPresent(flags, nFlags, '+')) {
		ADD_TO_BUF(buf, bufLen, ret, '+');
	} else if (flagPresent(flags, nFlags, ' ')) {
		ADD_TO_BUF(buf, bufLen, ret, ' ');
	}

	char temp[100];
	int nLen = 0;
	if (!in && precision == -1) {
		temp[0] = 0;
		nLen = 1;
	} else while (precision != 0) {
		char c = in % 16;
		in /= 16;
		if (c < 10) {
			c += '0';
		} else if (caps) {
			c += 'A' - 10;
		} else {
			c += 'a' - 10;
		}
		ADD_TO_BUF(temp, 100, nLen, c);
		if (precision == -1) {
			if (!in) {
				break;
			}
		} else {
			precision--;
		}
	}
	
	//TODO add width handling
	for (int i = nLen - 1; i >= 0; i--) {
		ADD_TO_BUF(buf, bufLen, ret, temp[i]);
	}
	return nLen;
}

static int printFloat(char *buf, int bufLen, float f) {
	int ret = 0;
	if (f < 0) {
		ADD_TO_BUF(buf, bufLen, ret, '-');
		f = -f;
	}
	unsigned long i = f;
	ret += printDec(&buf[ret], bufLen - ret, i, false, "", 0, -1, -1, 0);
	ADD_TO_BUF(buf, bufLen, ret, '.');
	f = (f - i) * 1000000 + 0.5;
	ret += printDec(&buf[ret], bufLen - ret, (unsigned long)f, false, "", 0, -1, 6, 0);
	return ret;
}

int vfprintf(FILE *stream, const char *format, va_list arg) {
	char buf[512];
	int len = 0;
	int error = 0;

	for (const char *c = format; *c != 0; c++) {
		if (*c != '%') {
			buf[len++] = *c;
			continue;
		}
		c++;

		char flags[5];
		int nFlags = 0;
		int width = -1;
		int precision = -1;
		char varLength = 0;

		//flags
		for (int i = 0; i < 5; i++) {
			if (isFlag(*c)) {
				flags[nFlags++] = *c;
				c++;
			} else {
				break;
			}
		}
		//width
		if (isdigit(*c)) {
			//parse width
		} else if (*c == '*') {
			width = va_arg(arg, int);
			c++;
		}
		//precision
		if (*c == '.') {
			c++;
			if (isdigit(*c)) {

			} else if (*c == '*') {
				precision = va_arg(arg, int);
				c++;
			}
			//invalid
		}
		//varLength
		if (*c == 'h' || *c == 'I' || *c == 'L') {
			varLength = *c;
			c++;
		}

		unsigned long in;
		const char *s;
		float f;
		switch (*c) {
			case 0:
				errno = EINVAL;
				return -1;
			case '%':
				ADD_TO_BUF(buf, 512, len, '%');
				break;
			case 'c':
				in = va_arg(arg, unsigned long);
				ADD_TO_BUF(buf, 512, len, in);
				break;
			case 's':
				s = va_arg(arg, const char *);
				while (*s) {
					ADD_TO_BUF(buf, 512, len, *s);
					s++;
				}
				break;
			case 'n':
				break; //print nothing
			case 'd':
			case 'i':
				in = va_arg(arg, unsigned long);
				error = printDec(buf + len, 512 - len, in, true, flags, nFlags, width, precision, varLength);
				if (error < 0) {
					errno = -error;
					return -1;
				}
				len += error;
				break;
			case 'u':
				in = va_arg(arg, unsigned long);
				error = printDec(buf + len, 512 - len, in, false, flags, nFlags, width, precision, varLength);
				if (error < 0) {
					errno = -error;
					return -1;
				}
				len += error;
				break;

			case 'x':
				in = va_arg(arg, unsigned long);
				error = printHex(buf + len, 512 - len, in, false, flags, nFlags, width, precision, varLength);
				if (error < 0) {
					errno = -error;
					return -1;
				}
				len += error;
				break;
			case 'p':
				varLength = 'l';
				//Fall through
			case 'X':
				in = va_arg(arg, unsigned long);
				error = printHex(buf + len, 512 - len, in, true, flags, nFlags, width, precision, varLength);
				if (error < 0) {
					errno = -error;
					return -1;
				}
				len += error;
				break;
			case 'f':
				f = va_arg(arg, double);
				error = printFloat(buf + len, 512 - len, f);
				if (error < 0) {
					errno = -error;
					return -1;
				}
				len += error;
				break;
			default:
				errno = EINVAL;
				return -1;
		}
	}
	buf[len] = 0;
	//error = fputs(buf, stream);
	error = fwrite(buf, 1, len, stream);
	if (!error) {
		error = len;
	}
	return error;
}

int fprintf(FILE *stream, const char *format, ...) {
	va_list args;
	va_start(args, format);

	int error = vfprintf(stream, format, args);

	va_end(args);
	return error;
}

int printf(const char *format, ...) {
	va_list args;
	va_start(args, format);

	int error = vfprintf(stdout, format, args);

	va_end(args);
	return error;
}