#include <stdio.h>
#include <string.h>
#include <errno.h>

#define BUFSIZE	512

char buf[BUFSIZE];

int main(int argc, char **argv) {
	FILE *f;
	if (argc < 2) {
		f = stdin;
	} else {
		f = fopen(argv[1], "r");
		if (!f) {
			printf("cat: %s\n", strerror(errno));
			return 1;
		}
	}
	int error = 0;
	while (!feof(f)) {
		size_t ret = fread(buf, 1, BUFSIZE, f);
		if (ret == 0) {
			if (errno) {
				printf("cat: %s\n", strerror(errno));
				error = 1;
			}
			break;
		}
		fwrite(buf, 1, ret, stdout);
	}

	if (argc >= 2) {
		fclose(f);
	}
	return error;
}