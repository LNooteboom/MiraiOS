#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <uapi/syscalls.h>
#include <sys/wait.h>

extern FILE *_PHFirstFile;
extern FILE *_PHLastFile;

FILE *popen(const char *command, const char *mode) {
	int commandLen = strlen(command);
	char *newArgv[64];
	char commandBuf[commandLen];

	memcpy(commandBuf, command, commandLen);
	commandBuf[commandLen] = 0; //omit newline character

	int newArgc = 1;
	newArgv[0] = commandBuf;
	for (int i = 0; i < commandLen; i++) {
		if (commandBuf[i] == ' ') {
			commandBuf[i] = 0;
			if (commandBuf[i + 1]) {
				newArgv[newArgc++] = &commandBuf[i + 1];
				if (newArgc == 64) {
					errno = E2BIG;
					return NULL;
				}
			}
		}
	}
	newArgv[newArgc] = NULL;

	int pipe[2];
	if (*mode != ' ') {
		sysPipe(pipe, 0);
	}
	

	fflush(stdout);
	pid_t child = fork();
	if (!child) {
		switch(*mode) {
		case 'r':
			sysClose(pipe[0]); //close reading end
			sysClose(STDOUT_FILENO);
			sysDup(pipe[1], STDOUT_FILENO, 0);
			sysClose(pipe[1]);
			break;
		case 'w':
			sysClose(pipe[1]); //close write end
			sysClose(STDIN_FILENO);
			sysDup(pipe[0], STDIN_FILENO, 0);
			sysClose(pipe[0]);
		}

		execvp(commandBuf, newArgv);
		exit(-1);
	}
	if (*mode != ' ') {
		return (void *)(long)child;
	}

	FILE *f = malloc(sizeof(*f));
	if (*mode == 'r') {
		sysClose(pipe[1]);
		f->fd = pipe[0];
	} else /*if (*mode == 'w')*/ {
		sysClose(pipe[0]);
		f->fd = pipe[1];
	}

	f->pid = child;
	f->buf = malloc(BUFSIZ);
	f->bufEnd = f->buf + BUFSIZ;
	f->writeEnd = f->buf;
	f->readEnd = f->buf;
	f->flags = _IOFBF | (1 << 2) | (1 << 5);
	f->cbuf = EOF;

	f->prev = _PHLastFile;
	_PHLastFile->next = f;
	_PHLastFile = f;

	return f;
}

int pclose(FILE *stream) {
	int status;

	fflush(stream);
	waitpid(stream->pid, &status, 0);

	stream->flags &= ~(1 << 5);
	fclose(stream);

	return status;
}

int system(const char *command) {
	if (!command) {
		return 1;
	}
	pid_t child = (pid_t)(long)popen(command, " ");
	int ret = 127;
	if (!child) {
		return ret;
	}
	waitpid(child, &ret, 0);
	return ret;
}