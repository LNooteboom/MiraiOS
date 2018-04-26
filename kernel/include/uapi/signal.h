#ifndef __PHLIBC_UAPI_SIGNAL_H
#define __PHLIBC_UAPI_SIGNAL_H

//Portable signal numbers, do not change
#define SIGHUP		1
#define SIGINT		2
#define SIGQUIT		3
#define SIGTRAP		5
#define SIGABRT		6
#define SIGKILL		9
#define SIGALRM		14
#define SIGRTERM	15

//os-specific signal numbers
#define SIGILL		4
#define SIGSTOP		7
#define SIGFPE		8
#define SIGSEGV		11
#define SIGPIPE		13

#define SIGRTMAX	63

#define SIG_DFL		((void *)0)
#define SIG_IGN		((void *)-1)

#define SIG_BLOCK	1
#define SIG_UNBLOCK	2
#define SIG_SETMASK	3

typedef unsigned long long	sigset_t;

union sigval {
	int sival_int;
	void *sival_ptr;
};

typedef struct {
	int si_signo;
	int si_errno;
	int si_code;
	int si_status;

	union {
		//sysKill()
		struct {
			pid_t si_pid;
			uid_t si_uid;
			union sigval si_value;
		} kill;

		//SIGCHLD
		struct {
			pid_t si_pid;
			uid_t si_uid;
		} chld;

		//SIGSEGV & friends
		void *si_addr;

	};
} siginfo_t;

struct sigaction {
	union {
		void (*sa_handler)(int);
		void (*sa_sigaction)(int, siginfo_t *, void *);
	};
	sigset_t sa_mask;
	int sa_flags;

	void (*saTrampoline)(int, siginfo_t *, void *);
};

#endif