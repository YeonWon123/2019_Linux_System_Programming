#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include "ssu_runtime.h"

#define ssu_show(limit) ssu_show_limit(limit, #limit)

void ssu_show_limit(int rlim_type, char *rlim_name);

int main(void)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	printf("	resource soft limit hard limit	\n\n");
	ssu_show(RLIMIT_CPU);
	ssu_show(RLIMIT_FSIZE);
	ssu_show(RLIMIT_DATA);
	ssu_show(RLIMIT_STACK);
	ssu_show(RLIMIT_CORE);
#ifdef RLIMIT_RSS
	ssu_show(RLIMIT_RSS);
#endif
#ifdef RLIMIT_MEMLOCK
	ssu_show(RLIMIT_MEMLOCK);
#endif
#ifdef RLIMIT_NPROC
	ssu_show(RLIMIT_NPROC);
#endif
#ifdef RLIMIT_OFILE
	ssu_show(RLIMIT_OFILE);
#endif
#ifdef RLIMIT_NOFILE
	ssu_show(RLIMIT_NOFILE);
#endif
#ifdef RLIMIT_VMEN
	ssu_show(RLIMIT_VMEN);
#endif

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}

void ssu_show_limit(int rlim_type, char *rlim_name) {
	struct rlimit rlim;
	char cur[11], max[11];

	getrlimit(rlim_type, &rlim);

	if (rlim.rlim_cur == RLIM_INFINITY)
		strcpy(cur, "infinity");
	else
		sprintf(cur, "%10ld", rlim.rlim_cur);

	if (rlim.rlim_max == RLIM_INFINITY)
		strcpy(max, "infinity");
	else
		sprintf(max, "%10ld", rlim.rlim_max);

	printf("%15s : %10s %10s\n", rlim_name, cur, max);
}
