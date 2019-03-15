#include <uapi/syscalls.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char **argv) {
	if (argc == 1) {
		fprintf(stderr, "Error: specify -h or -r\n");
		return 1;
	}
	uint32_t mode;
	if (!strcmp(argv[1], "-r")) {
		mode = POWER_REBOOT;
	} else if (!strcmp(argv[1], "-h")) {
		mode = POWER_SHUTDOWN;
	} else {
		fprintf(stderr, "Error: invalid mode %s\n", argv[1]);
		return 1;
	}
	int err = sysPower(POWER_MAGIC, mode);
	fprintf(stderr, "Error: %d %s\n", -err, strerror(-err));
	return 1;
}