#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

static long parseInt(const char *s, const char *end) {
	long ret = 0;
	while (s != end) {
		ret *= 10U;
		ret += *s - '0';
		s++;
	}
	return ret;
}

double strtod(const char *s, char **endp) {
	while (isspace(*s)) {
		s++;
	}

	bool neg = false;
	if (*s == '-') {
		neg = true;
		s++;
	} else if (*s == '+') {
		s++;
	} else if (!isdigit(*s) && *s != '.') {
		errno = EINVAL;
		return 0.0;
	}

	const char *radix = NULL;
	const char *expSym = NULL;
	const char *end = s;
	
	//layout should be: <int> radix <frac> expSym <+/-><exp>
	while (*end) {
		if (*end == '.') {
			if (!radix && !expSym) {
				radix = end;
			} else {
				errno = EINVAL;
				return 0.0;
			}
		} else if (*end == 'E' || *end == 'e') {
			if (!expSym) {
				expSym = end;
			} else {
				errno = EINVAL;
				return 0.0;
			}
		} else if (!isdigit(*end)) {
			break;
		}
		end++;
	}
	if (endp){
		*endp = (char *)end;
	}
	
	if (!expSym) {
		expSym = end;
	}
	if (!radix) {
		radix = expSym;
	}

	double ret = parseInt(s, radix);
	//parse fraction
	const char *i = radix + 1;
	if (radix != expSym) {
		double div = 10.0;
		while (i != expSym) {
			ret += (*i - '0') / div;
			div *= 10;
			i++;
		}
	}

	if (neg) {
		ret = -ret;
	}

	if (expSym == end) {
		return ret;
	}

	i = expSym + 1;
	bool expNeg = false;
	if (*i == '-') {
		expNeg = true;
		i++;
	} else if (*i == '+' || isdigit(*i)) {
		if (*i == '+') {
			i++;
		}
	} else {
		return ret;
	}
	long exp = parseInt(i, end);
	float mul = 10;
	float retmul = 1;
	while (exp) {
		if (exp & 1) {
			retmul *= mul;
		}
		mul = mul * mul;
		exp >>= 1;
	}
	if (expNeg) {
		ret /= retmul;
	} else {
		ret *= retmul;
	}
	return ret;
}