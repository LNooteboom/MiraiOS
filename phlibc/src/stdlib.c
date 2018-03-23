#include <stdlib.h>
#include <unistd.h>
#include <string.h>

char *getenv(const char *name) {
	char *curEnv = *environ;
	int i = 1;
	int nameLen = strlen(name);
	while (curEnv) {
		if (!memcmp(name, curEnv, nameLen) && curEnv[nameLen] == '=') {
			return &curEnv[nameLen + 1];
		}
		curEnv = environ[i++];
	}
	return NULL;
}