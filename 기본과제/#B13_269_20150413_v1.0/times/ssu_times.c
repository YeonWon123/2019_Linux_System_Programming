#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <sys/wait.h>

void ssu_do_cmd(char *cmd);
void ssu_print_times(clock_t real, struct tms *tms_start, struct tms *tms_end);
void ssu_echo_exit(int status);

int main(int argc, char *argv[])
{
	int i;
	
	setbuf(stdout, NULL);

	for (i = 1; i < argc; i++)
		ssu_do_cmd(argv[i]);

	exit(0);
}

void ssu_do_cmd(char *cmd) {
	struct tms tms_start, tms_end;
	clock_t start, end;
	int status;

	printf("\ncommand: %s\n", cmd);

	if ((start = times(&tms_start)) == -1) {
		fprintf(stderr, "times error\n");
		exit(1);
	}

	if ((status = system(cmd)) < 0) {
		fprintf(stderr, "system error\n");
		exit(1);
	}

	if ((end = times(&tms_end)) == -1) {
		fprintf(stderr, "times error\n");
		exit(1);
	}

	ssu_print_times(end - start, &tms_start, &tms_end);
	ssu_echo_exit(status);
}

void ssu_print_times(clock_t real, struct tms *tms_start, struct tms *tms_end) {
	static long clocktick = 0;

	if (clocktick == 0)
		if ((clocktick = sysconf(_SC_CLK_TCK)) < 0) {
			fprintf(stderr, "sysconf error\n");
			exit(1);
		}

	printf("	real: %7.2f\n", real / (double) clocktick);
	printf("	user: %7.2f\n", (tms_end->tms_utime - tms_start->tms_utime) / (double) clocktick);
	printf("	sys : %7.2f\n", (tms_end->tms_stime - tms_start->tms_stime) / (double) clocktick);
	printf("	child user: %7.2f\n", (tms_end->tms_cutime - tms_start->tms_cutime) / (double) clocktick);
	printf("	child sys : %7.2f\n", (tms_end->tms_cstime - tms_start->tms_cstime) / (double) clocktick);
}

void ssu_echo_exit(int status) {
	if (WIFEXITED(status))
		printf("normal termination, exit status  = %d\n", WEXITSTATUS(status));
	else if (WIFSIGNALED(status))
		printf("abnormal termination, signal number = %d%s\n", WTERMSIG(status),
#ifdef WCOREDUMP
	WCOREDUMP(status) ? " (core file generated)" : "");
#else
	"");
#endif
	else if (WIFSTOPPED(status))
		printf("child stopped, signal number = %d\n", WSTOPSIG(status));
}
