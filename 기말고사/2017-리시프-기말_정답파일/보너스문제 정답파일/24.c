#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>

#define BUF_SIZE 2048
#define STDIN 0
#define STDOUT 1
#define NUM_ALPHA 26

int alpha_proc[NUM_ALPHA];
int pnc_pipe1[NUM_ALPHA][2];
int pnc_pipe2[NUM_ALPHA][2];
char *init_str = "Linux System Programming Bonus!\n";
char *end_str = "End of Bonus!\n";
int term_fd;

void ssu_atexit(void);
void ssu_sigusr1_handler(int signo);

int main(int argc, char *argv[])
{
	int termpipe_fd[2], i;
	char cur_term[BUF_SIZE] = {0};
	char c, tmp;
	pipe(termpipe_fd);
	close(STDOUT);
	
	/*
	   에러처리에는 stderr을 사용하나, stderr으로 출력하면 안됨
		your code start here!
		...
	*/

	if (fork() == 0) {
		dup(termpipe_fd[1]);
		execl("/usr/bin/tty", "tty", NULL);
	}
	else {
		close(STDIN);
		dup(termpipe_fd[0]);
		scanf("%s", cur_term);

		if ((term_fd = open(cur_term, O_RDWR)) < 0) {
			fprintf(stderr, "open error for %s\n", cur_term);
			exit(1);
		}
		write(term_fd, init_str, strlen(init_str));
	}

	atexit(ssu_atexit);
	signal(SIGUSR1, ssu_sigusr1_handler);

	for (i = 0; i < NUM_ALPHA; i++) {
		if (pipe(pnc_pipe1[i]) < 0 || pipe(pnc_pipe2[i]) < 0) {
			fprintf(stderr, "pipe error\n");
			exit(1);
		}

		if ((alpha_proc[i] = fork()) < 0) {
			fprintf(stderr, "fork error\n");
			exit(1);
		}
		else if (alpha_proc[i] == 0) {
			while (1) {
				while (read(pnc_pipe1[i][0], &c, 1) <= 0);		// busy waiting
				tmp = 'a' + i;
				write(term_fd, &tmp, 1);
				while (write(pnc_pipe2[i][1], &c, 1) <= 0);		
			}
		}
	}

	for (i = 0; argv[1][i] != 0; i++) {
		if (argv[1][i] == ' ') {
			tmp = ' ';
			write(term_fd, &tmp, 1);
			continue;
		}
		while (write(pnc_pipe1[argv[1][i] - 'a'][1], &c, 1) <= 0);
		while (read(pnc_pipe2[argv[1][i] - 'a'][0], &c, 1) <= 0);	// busy waiting
	}

	write(term_fd, "\n", 1);

	exit(0);
}

void ssu_atexit(void) {
	int i;
	for (i = 0; i < NUM_ALPHA; i++)
		kill(alpha_proc[i], SIGUSR1);
	write(term_fd, end_str, strlen(end_str));
}

void ssu_sigusr1_handler(int signo) {
	_exit(0);
}
