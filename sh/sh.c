#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>

#define COMMAND_BUF_LEN	512

char commandBuf[COMMAND_BUF_LEN];

char *newArgv[32];

extern void _PHSigTramp(void);

void handler(int sig) {
	printf("\n%s", getenv("PS1"));
}

int main(void) {
	puts("MiraiOS Shell v0.1");

	signal(SIGINT, handler);

	pid_t curPgid = getpgid(0);

	while (1) {
		ioctl(STDIN_FILENO, TIOCSPGRP, &curPgid);
		printf("%s", getenv("PS1"));
		fgets(commandBuf, COMMAND_BUF_LEN, stdin);

		int commandLen = strlen(commandBuf);
		commandBuf[--commandLen] = 0; //omit newline character
		if (!commandLen) {
			continue;
		}

		int newArgc = 1;
		newArgv[0] = commandBuf;
		for (int i = 0; i < commandLen; i++) {
			if (commandBuf[i] == ' ') {
				commandBuf[i] = 0;
				if (commandBuf[i + 1]) {
					newArgv[newArgc++] = &commandBuf[i + 1];
				}
			}
		}
		newArgv[newArgc] = NULL;

		if (!memcmp(commandBuf, "cd", 3)) {
			if (newArgc == 1) {
				chdir("/");
				continue;
			}
			chdir(newArgv[1]);
			continue;
		} else if (!memcmp(commandBuf, "exit", 5)) {
			puts("Exit");
			exit(0);
		}

		fflush(stdout);
		pid_t child = fork();
		if (!child) {
			setpgid(0, 0);
			pid_t grp = getpgid(0);
			ioctl(STDIN_FILENO, TIOCSPGRP, &grp);
			execvp(commandBuf, newArgv);
			if (errno == -ENOENT) {
				printf("%s: Command not found\n", commandBuf);
			}
			exit(-1);
		}
		int status = 10;
		waitpid(0, &status, 0);
	}

	return 0;
}