#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("touch: Need filename\n");
	}

	int error = 0;
	for (int i = 1; i < argc; i++) {
		FILE *f = fopen(argv[i], "w");
		if (!f) {
			printf("touch: %s\n", strerror(errno));
			error = errno;
			break;
		}
		fclose(f);
	}
	return error;
}