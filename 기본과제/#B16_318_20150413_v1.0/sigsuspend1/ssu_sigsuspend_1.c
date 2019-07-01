#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{
	sigset_t old_set;
	sigset_t sig_set;

	sigemptyset(&sig_set);
	sigaddset(&sig_set, SIGINT);
	sigprocmask(SIG_BLOCK, &sig_set, &old_set);
	sigsuspend(&old_set);
	exit(0);
}
