#include <uapi/syscalls.h>
#include <uapi/fcntl.h>

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

char str[] = "\e[47;31mHello From C!!!\n\e[0m";

char *env[] = {
	"PATH=/bin",
	"USELESS=useless",
	NULL
};

int main(void) {
	sysOpen("/dev/tty0", SYSOPEN_FLAG_READ); //stdin
	sysOpen("/dev/tty0", SYSOPEN_FLAG_WRITE); //stdout
	sysOpen("/dev/tty0", SYSOPEN_FLAG_READ | SYSOPEN_FLAG_WRITE); //stderr
	//sysWrite(0, str, sizeof(str) - 1);
	puts(str);

	environ = env;
	printf("env: %s\n", getenv("USELESS"));

	while (1) {
		//if (sysWaitPid(0, NULL, 0)) {
			sysSleep(1, 0);
		//}
	}

	return 0;
}
