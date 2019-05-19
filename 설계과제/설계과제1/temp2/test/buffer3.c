#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h> 

struct part {
	char name[1024];
	int name_s;
	char type[10];
	int num;
	int name_i;
};

int main(void)
{
	FILE *fp;
	struct part num[50];
 
	int innerFileCount = 50;
	char studentFile[50][50];
	int m;
	char *text_f;

	fp = fopen("score.txt", "r+");
	for(m = 0; m < innerFileCount; m++) {
		fscanf(fp, "%s", studentFile[m]);
	}

	int count1 = 0;
	for(m = 0; m < innerFileCount; m++) {
		printf("%s\n", studentFile[m]);
		count1++;
	}
	printf("총 count 개수 : %d\n", count1);

	int count2 = 0;
	for(m = 0; m < innerFileCount; m++) {
		printf("\\\\\%s\n", studentFile[m]);
		strcpy(num[m].name, studentFile[m]);
		num[m].num = m;
		char* test_l = (char*)malloc(1000);
		strcpy(test_l, studentFile[m]);
		text_f = strtok(test_l, ".-");

		while (text_f != NULL) {
			text_f = strtok(NULL, ".");
			if (strcmp(text_f, "txt") == 0) {
				strcpy(num[m].type, "txt");					
				break;
			}
			else if (strcmp(text_f, "c") == 0) {
				num[m].name_s = 0;
				strcpy(num[m].type, "c");
				break;
			}
			num[m].name_s = atoi(text_f);
		}
		num[m].name_i = atoi(studentFile[m]);
		count2++;
	}

	printf("총 count 개수 : %d\n", count2);
	return 0;
}
