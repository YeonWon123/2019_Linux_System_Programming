#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

void ssu_signal_handler(int signo);
void ssu_timestamp(char *str);

int main(void)
{
    struct sigaction sig_act;
    sigset_t blk_set;
    pid_t pid;

    sigfillset(&blk_set);
    sigdelset(&blk_set, SIGCHLD);
    sigemptyset(&sig_act.sa_mask);
    sig_act.sa_flags = 0;
    sig_act.sa_handler = ssu_signal_handler;
    sigaction(SIGCHLD, &sig_act, NULL);

    printf("before fork\n");
    if ((pid = fork()) < 0) {
	fprintf(stderr, "fork error\n");
	exit(1);
    } else if (pid == 0) {
	printf("after fork in child, sleep...\n");
	sleep(3);
    } else {
	printf("after fork in parent, suspend...\n");
	sigsuspend(&blk_set);
	printf("after suspend\n");
    }
    exit(0);
}

void ssu_signal_handler(int signo) {
    printf("in ssu_signal_handler() function\n");
}
