#include <stdio.h>
#include <uapi/syscalls.h>
#include <uapi/fcntl.h>
#include <uapi/getDent.h>

int main(int argc, char **argv) {
	char *dir;
	if (argc == 2) {
		dir = argv[1];
	} else {
		dir = ".";
	}
	int dirFD = sysOpen(dir, SYSOPEN_FLAG_READ | SYSOPEN_FLAG_DIR);
	struct GetDent dent;
	while (1) {
		int err = sysGetDent(dirFD, &dent);
		if (err <= 0) {
			break;
		}
		if (dent.type == 1) {
			printf("\e[32m%s\e[0m\n", dent.name);
		} else {
			printf("%s\n", dent.name);
		}
		
	}

	return 0;
}