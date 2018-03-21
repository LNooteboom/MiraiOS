#include <uapi/syscalls.h>
#include <uapi/fcntl.h>
#include <uapi/mmap.h>

char str[] = "\e[47;31mHello From C!!!\n\e[0m";

int main(void) {
	sysOpen("/dev/tty0", SYSOPEN_FLAG_WRITE | SYSOPEN_FLAG_READ);
	sysWrite(0, str, sizeof(str) - 1);

	while (1) {
		sysSleep(1, 0);
	}

	return 0;
}
