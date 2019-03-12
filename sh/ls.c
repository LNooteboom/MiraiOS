#include <stdio.h>
#include <dirent.h>
#include <errno.h>

#include <uapi/syscalls.h>
#include <uapi/stat.h>
#include <string.h>

char nameBuf[256];

int main(int argc, char **argv) {
	char *name;
	if (argc == 2) {
		name = argv[1];
	} else {
		name = ".";
	}

	DIR *dir = opendir(name);
	if (!dir) {
		return errno;
	}

	strcpy(nameBuf, name);
	char *d = nameBuf + strlen(nameBuf);
	*d++ = '/';

	while (1) {
		struct dirent *entry = readdir(dir);
		if (!entry) {
			return errno;
		}

		strcpy(d, entry->d_name);
		struct stat stbuf;
		sysStat(nameBuf, &stbuf);

		if (entry->d_type == DT_DIR) {
			printf("\e[32m%s\e[0m\n", entry->d_name);
		} else {
			printf("%s %d\n", entry->d_name, stbuf.st_ino);
		}
	}
}