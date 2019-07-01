#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void ssu_check_pending(int signo, char *signame);
void ssu_signal_handler(int signo);

int main(void)
{
	struct sigaction sig_act;
	sigset_t sig_set;

	sigemptyset(&sig_act.sa_mask);
	sig_act.sa_flags = 0;
	sig_act.sa_handler = ssu_signal_handler;

	if (sigaction(SIGUSR1, &sig_act, NULL) != 0) {
		fprintf(stderr, "sigaction() error\n");
		exit(1);
	}
	else {
		sigemptyset(&sig_set);
		sigaddset(&sig_set, SIGUSR1);

		if (sigprocmask(SIG_SETMASK, &sig_set, NULL) != 0) {
			fprintf(stderr, "sigprocmask() error\n");
			exit(1);
		}
		else {
			printf("SIGUSR1 signals are now blocked\n");
			kill(getpid(), SIGUSR1);
			printf("after kill()\n");

			ssu_check_pending(SIGUSR1, "SIGUSR1");
			sigemptyset(&sig_set);
			sigprocmask(SIG_SETMASK, &sig_set, NULL);
			printf("SIGUSR1 signals are no longer blocked\n");
			ssu_check_pending(SIGUSR1, "SIGUSR1");
		}
	}

	exit(0);
}

void ssu_check_pending(int signo, char *signame) {
	sigset_t sig_set;

	if (sigpending(&sig_set) != 0)
		printf("sigpending() error\n");
	else if (sigismember(&sig_set, signo))
		printf("a %s signal is pending\n", signame);
	else
		printf("%s signals are not pending\n", signame);
}

void ssu_signal_handler(int signo) {
	printf("in ssu_signal_handler function\n");
}
