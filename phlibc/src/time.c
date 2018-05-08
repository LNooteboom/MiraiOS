#include <time.h>

static struct tm tmr;
static long timeVar = 1525702402;


time_t time(time_t *ptr) {
	time_t ret = timeVar;
	timeVar += 1;
	if (ptr) {
		*ptr = ret;
	}
	return ret;
}

time_t mktime(struct tm *timeptr) {
	return timeVar++;
}

double difftime(time_t time1, time_t time0) {
	return 0;
}

clock_t clock(void) {
	return timeVar * CLOCKS_PER_SEC;
}

struct tm *gmtime(const time_t *timer) {
	return &tmr;
}

struct tm *gmtime_r(const time_t *restrict timer, struct tm *restrict result) {
	return &tmr;
}

struct tm *localtime(const time_t *timer) {
	return &tmr;
}

struct tm *localtime_r(const time_t *restrict timer, struct tm *restrict result) {
	return &tmr;
}

size_t strftime(char *restrict s, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr) {
	return 0;
}


long random(void) {
	return timeVar++; //TODO
}
void srandom(unsigned int seed) {
}
int rand(void) {
	return timeVar++;
}
void srand(unsigned int seed) {
}