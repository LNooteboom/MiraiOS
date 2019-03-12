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
	if (getpid() != 1) {
		exit(1);
	}

	sysOpen(AT_FDCWD, "/dev/tty0", SYSOPEN_FLAG_READ); //stdin
	sysOpen(AT_FDCWD, "/dev/tty0", SYSOPEN_FLAG_WRITE); //stdout
	sysOpen(AT_FDCWD, "/dev/tty0", SYSOPEN_FLAG_READ | SYSOPEN_FLAG_WRITE); //stderr

	sysOpen(AT_FDCWD, "/tmp", SYSOPEN_FLAG_CREATE | SYSOPEN_FLAG_DIR);


	environ = env;
	pid_t sh = fork();
	if (!sh) {
		setpgid(0, 0);
		execlp("sh", "sh", NULL);
	}
	sysWaitPid(sh, NULL, 0);

	while (1) {
		//if (sysWaitPid(0, NULL, 0)) {
			sysSleep(1, 0);
		//}
	}

	return 0;
}
