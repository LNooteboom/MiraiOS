#include <stdio.h>
#include <string.h>
#include <uapi/syscalls.h>
#include <uapi/fcntl.h>

int main(int argc, char **argv) {
	if (argc < 2) {
		printf("mkdir: Need directory name\n");
	}

	int error = 0;
	for (int i = 1; i < argc; i++) {
		error = sysOpen(AT_FDCWD, argv[i], SYSOPEN_FLAG_CREATE | SYSOPEN_FLAG_EXCL
			| SYSOPEN_FLAG_DIR | SYSOPEN_FLAG_READ);
		if (error < 0) {
			error = -error;
			printf("rm: %s\n", strerror(error));
			break;
		}
		sysClose(error);
	}
	return error;
}