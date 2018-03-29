#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		if (i != 1) {
			putc(' ', stdout);
		}
		fputs(argv[i], stdout);
	}
	putc('\n', stdout);
	return 0;
}