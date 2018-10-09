#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <uapi/syscalls.h>

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
		/*for (size_t i = 0; i < ret; i++) {
			sysWrite(1, &buf[i], 1);
			//sysSleep(0, 2000000);
		}*/
	}

	if (argc >= 2) {
		fclose(f);
	}
	return error;
}