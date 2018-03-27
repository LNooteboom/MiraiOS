#include <uapi/syscalls.h>
#include <uapi/fcntl.h>

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

char *env[] = {
	"PATH=/bin",
	"PS1=\e[31mMirai\e[37mOS\e[0m> ",
	NULL
};

int main(void) {
	sysOpen("/dev/tty1", SYSOPEN_FLAG_READ); //stdin
	sysOpen("/dev/tty1", SYSOPEN_FLAG_WRITE); //stdout
	sysOpen("/dev/tty1", SYSOPEN_FLAG_READ | SYSOPEN_FLAG_WRITE); //stderr

	environ = env;
	pid_t sh = sysFork();
	if (!sh) {
		//execvpe("sh", environ, environ);
		execlp("sh", "sh", "test", NULL);
	}
	sysWaitPid(sh, NULL, 0);

	while (1) {
		//if (sysWaitPid(0, NULL, 0)) {
			sysSleep(1, 0);
		//}
	}

	return 0;
}
