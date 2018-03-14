#include <syscalls.h>

char str[] = "\e[47;31mHello From C!!!\n\e[0m";
char child[] = "Child active\n";

int main(void) {
	//while(1);
	sysOpen("/dev/tty0", SYSOPEN_FLAG_WRITE);

	int *sem = sysMmap(NULL, 0x1000, MMAP_FLAG_WRITE | MMAP_FLAG_ANON | MMAP_FLAG_SHARED, 0, 0);
	*sem = 0;

	sysWrite(0, str, sizeof(str) - 1);
	if (!sysFork()) {
		sysWrite(0, child, sizeof(child) - 1);
		*sem = 1;
		sysExit(0);
	}
	while (!(*sem));
	sysWrite(0, str, sizeof(str) - 1);

	while (1) {
		sysSleep(1, 0);
	}

	return 0;
}
