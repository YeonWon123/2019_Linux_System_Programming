#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	extern char **environ;
	char **str;
	int i;

	for (i = 0; i < argc; i++)
		printf("argv[%d]: %s\n", i, argv[i]);

	for (str = environ; *str != 0; str++)
		printf("%s\n", *str);

	exit(0);
}
