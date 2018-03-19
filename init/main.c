#include <uapi/syscalls.h>
#include <uapi/fcntl.h>
#include <uapi/mmap.h>

char str[] = "\e[47;31mHello From C!!!\n\e[0m";

int main(void) {
	sysOpen("/dev/tty0", SYSOPEN_FLAG_WRITE | SYSOPEN_FLAG_READ);
	sysWrite(0, str, sizeof(str) - 1);

	int pipefd[2];
	sysPipe(pipefd, 0);

	int new = sysDup(pipefd[0], 32, 0);

	sysWrite(pipefd[1], str, sizeof(str) - 1);
	char buf[64];
	sysRead(32, buf, sizeof(str) - 1);

	sysWrite(0, buf, sizeof(str) - 1);
	while (1) {
		sysSleep(1, 0);
	}

	return 0;
}
