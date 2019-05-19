#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

void ssu_signal_handler1(int signo);
void ssu_signal_handler2(int signo);

int main(void)
{
	if (signal(SIGINT, ssu_signal_handler1) == SIG_ERR) {
		fprintf(stderr, "cannot handle SIGINT\n");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGUSR1, ssu_signal_handler2) == SIG_ERR) {
		fprintf(stderr, "cannot handle SIGUSR1\n");
		exit(EXIT_FAILURE);
	}

	raise(SIGINT);
	raise(SIGUSR1);
	printf("main return\n");
	exit(0);
}

void ssu_signal_handler1(int signo) {
	printf("SIGINT 시그널 발생\n");
}

void ssu_signal_handler2(int signo) {
	printf("SIGUSR1 시그널 발생\n");
}
