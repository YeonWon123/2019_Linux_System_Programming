#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

void ssu_alarm(int signo);

int main(void)
{
	printf("Alarm Setting\n");
	signal(SIGALRM, ssu_alarm);
	alarm(2);

	while (1) {
		printf("done\n");
		pause();
		alarm(2);
	}

	exit(0);
}

void ssu_alarm(int signo) {
	printf("alarm..!!!\n");
}
