#include <stdio.h>
#include <dirent.h>
#include <errno.h>

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
	while (1) {
		struct dirent *entry = readdir(dir);
		if (!entry) {
			return errno;
		}
		if (entry->d_type == DT_DIR) {
			printf("\e[32m%s\e[0m\n", entry->d_name);
		} else {
			printf("%s\n", entry->d_name);
		}
	}
}