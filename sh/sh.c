#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <uapi/syscalls.h>

#define COMMAND_BUF_LEN	512

char commandBuf[COMMAND_BUF_LEN];

char *newArgv[32];

int main(void) {
	puts("MiraiOS Shell v0.1\n\n");

	while (1) {
		fputs(getenv("PS1"), stdout);

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
				sysChDir("/");
				continue;
			}
			sysChDir(newArgv[1]);
			continue;
		} else if (!memcmp(commandBuf, "exit", 5)) {
			puts("Exit");
			exit(0);
		}

		pid_t child = sysFork();
		if (!child) {
			execvp(commandBuf, newArgv);
			if (errno == -ENOENT) {
				printf("%s: Command not found\n", commandBuf);
			}
			exit(-1);
		}
		sysWaitPid(child, NULL, 0);
	}

	return 0;
}