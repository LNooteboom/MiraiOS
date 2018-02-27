#include <syscalls.h>

char str[] = "Hello From C!!!\n";
char end[] = "End!\n";

int main(void) {
	sysOpen("/dev/tty0", SYSOPEN_FLAG_WRITE);

	sysWrite(0, str, sizeof(str) - 1);

	int fd = sysOpen("/", SYSOPEN_FLAG_READ);
	struct GetDent buf;
	int nameLen;
	while ((nameLen = sysGetDent(fd, &buf)) > 0) {
		buf.name[nameLen] = ' ';
		sysWrite(0, buf.name, nameLen + 1);
	}

	sysWrite(0, end, sizeof(end) - 1);

	while (1) { //init cannot return
		sysSleep(0, 10000000);
	}
	return 0;
}