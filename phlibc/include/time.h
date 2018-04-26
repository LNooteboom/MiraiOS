#ifndef __PHLIBC_TIME_H
#define __PHLIBC_TIME_H

typedef long time_t;

time_t time(time_t *ptr) {
	time_t ret = 0xDEADBEEF;
	if (ptr) {
		*ptr = ret;
	}
	return ret;
}

#endif