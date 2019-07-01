#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

int main(void)
{
	sigset_t pendingset;
	sigset_t sig_set;
	int count = 0;

	sigfillset(&sig_set);
	sigprocmask(SIG_SETMASK, &sig_set, NULL);

	while (1) {
		printf("count: %d\n", count++);
		sleep(1);

		if (sigpending(&pendingset) == 0) {
			if (sigismember(&pendingset, SIGINT)) {
				printf("SIGINT가 블록되어 대기 중. 무한 루프를 종료.\n");
				break;
			}
		}
	}
	
	exit(0);
}
