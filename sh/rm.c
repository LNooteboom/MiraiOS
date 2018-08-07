#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("rm: Need filename\n");
	}

	int error = 0;
	for (int i = 1; i < argc; i++) {
		error = remove(argv[i]);
		if (error) {
			printf("rm: %s\n", strerror(errno));
			error = errno;
			break;
		}
	}
	return error;
}