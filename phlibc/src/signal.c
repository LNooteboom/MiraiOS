#include <uapi/syscalls.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

extern void _PHSigTramp(int sigNum, siginfo_t *info, void *context);

int sigaction(int sig, const struct sigaction *act, struct sigaction *oldact) {
	struct sigaction act2;
	memcpy(&act2, act, sizeof(act2));
	act2.saTrampoline = _PHSigTramp;
	int error = sysSigHandler(sig, &act2, oldact);
	if (error) {
		errno = -error;
		return -1;
	}
	return 0;
}

void (*signal(int sig, void (*func)(int)))(int) {
	struct sigaction act = {
		.sa_handler = func,
		.sa_mask = 0,
		.sa_flags = 0,
		.saTrampoline = _PHSigTramp
	};
	int error = sysSigHandler(sig, &act, NULL);
	if (error) {
		return SIG_ERR;
	}
	return func;
}