#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include <sys/wait.h>

double ssu_maketime(struct timeval *time);

void term_stat(int stat);

void ssu_print_child_info(int stat, struct rusage *rusage);

int main(void)
{
	struct rusage rusage;
	pid_t pid;
	int status;

	if ((pid = fork()) == 0) {
		char *args[] = {"find", "/", "-maxdepth", "4", "-name", "stdio.h", NULL};

		if (execv("/usr/bin/find", args) < 0) {
			fprintf(stderr, "execv error\n");
			exit(1);
		}
	}

	if (wait3(&status, 0, &rusage) == pid)
		ssu_print_child_info(status, &rusage);
	else {
		fprintf(stderr, "wait3 error\n");
		exit(1);
	}

	exit(0);
}

double ssu_maketime(struct timeval *time) {
	return ((double)time -> tv_sec + (double)time -> tv_usec/1000000.0);
}

void term_stat(int stat) {
	if (WIFEXITED(stat))
		printf("normally terminated. exit status = %d\n", WEXITSTATUS(stat));
	else if (WIFSIGNALED(stat))
		printf("abnormal termination by signal %d. %s\n", WTERMSIG(stat),
#ifdef WCOREDUMP
				WCOREDUMP(stat)?"core dumped":"no core"
#else
				NULL
#endif
		      );
	else if (WIFSTOPPED(stat))
		printf("stopped by signal %d\n", WSTOPSIG(stat));
}

void ssu_print_child_info(int stat, struct rusage *rusage) {
	printf("Termination info follows\n");
	term_stat(stat);
	printf("user CPU time : %.2f(sec)\n", ssu_maketime(&rusage->ru_utime));
	printf("system CPU time : %.2f(sec)\n", ssu_maketime(&rusage->ru_stime));
}
