#include <syscalls.h>

char str[] = "\e[47;31mHello From C!!!\n";

int main(void) {
	sysOpen("/dev/tty0", SYSOPEN_FLAG_WRITE);

	sysWrite(0, str, sizeof(str) - 1);

	while (1) { //init cannot return
		sysSleep(1, 0); //sleep for 1 second
	}
	return 0;
}