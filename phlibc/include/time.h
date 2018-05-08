#ifndef __PHLIBC_TIME_H
#define __PHLIBC_TIME_H

#include <stddef.h>

#define CLOCKS_PER_SEC	1000000

typedef long time_t;
typedef long clock_t;

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

time_t time(time_t *ptr);
time_t mktime(struct tm *timeptr);

double difftime(time_t time1, time_t time0);

clock_t clock(void);

struct tm *gmtime(const time_t *timer);
struct tm *gmtime_r(const time_t *restrict timer, struct tm *restrict result);

struct tm *localtime(const time_t *timer);
struct tm *localtime_r(const time_t *restrict timer, struct tm *restrict result);

size_t strftime(char *restrict s, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr);

#endif