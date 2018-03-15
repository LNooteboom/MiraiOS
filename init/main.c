#include <uapi/syscalls.h>
#include <uapi/fcntl.h>
#include <uapi/mmap.h>

char str[] = "\e[47;31mHello From C!!!\n\e[0m";
char child[] = "Child active\n";

int main(void) {
	//while(1);
	asm ("xchg %bx, %bx");
	sysOpen("/dev/tty0", SYSOPEN_FLAG_WRITE | SYSOPEN_FLAG_READ);

	sysWrite(0, str, sizeof(str) - 1);
	char in[8];
	int n = sysRead(0, in, 8);
	str[11] = n + '0';
	sysWrite(0, str, sizeof(str) - 1);
	sysWrite(0, in, n);

	while (1) {
		sysSleep(1, 0);
	}

	return 0;
}
