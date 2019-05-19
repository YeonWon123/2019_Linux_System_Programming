#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

static void ssu_signal_handler(int signo);

int main(void)
{
	if (signal(SIGINT, ssu_signal_handler) == SIG_ERR) {
		fprintf(stderr, "cannot handle SIGINT\n");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGTERM, ssu_signal_handler) == SIG_ERR) {
		fprintf(stderr, "cannot handle SIGTERM\n");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGPROF, SIG_DFL) == SIG_ERR) {
		fprintf(stderr, "cannot reset SIGPROF\n");
		exit(EXIT_FAILURE);
	}

	if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
		fprintf(stderr, "cannot ignore SIGHUP\n");
		exit(EXIT_FAILURE);
	}

	while(1)
		pause();

	exit(0);
}

static void ssu_signal_handler(int signo) {
	if (signo == SIGINT)
		printf("caught SIGINT\n");
	else if (signo == SIGTERM)
		printf("caught SIGTERM\n");
	else {
		fprintf(stderr, "unexpected signal\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
