#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
//#include "ssu_runtime.h"

#define MAX_SIZE 100

//total question number
struct ans_num {
	char* name;
	int name_i;
	int name_s;
	int num;
	char* type;
};

int compare(const void*a, const void*b) {
	return strcmp(*(char**)a, *(char**)b);
}

void ans_numSort(struct ans_num* num);
void subdirOutput(char *wd, char* studentId, char*wd_a, char*answerQ);

//....
char* studentFile[MAX_SIZE];
int innerFileCount;

int main(int argc, char **argv)
{
	//-----
//	struct timeval begin_t, end_t;
//	gettimeofday(&begin_t, NULL);
	//---time

	//<---
	char* studentId[MAX_SIZE];
	char* answerQ[MAX_SIZE];

	//student
	DIR *dir = opendir(argv[1]);
	if (dir == NULL) {
		printf("first directory failed open\n");
		exit(1);
	}

	struct dirent *de = NULL;
	struct dirent *subde = NULL;
	int i, j;
	int num_s = 0;
	int num_a = 0;

	//STD_DIR folder name save
	while ((de = readdir(dir)) != NULL) {

		//save
		if (num_s >= 100) {
			printf("can not save\n");
		}
		if ((!strcmp(de->d_name, ".")) || (!strcmp(de->d_name, "..")))
			continue;

		studentId[num_s] = de->d_name;
		num_s++;
	}
	//STD_DIR folder name sort	
	qsort(studentId, num_s, sizeof(studentId[0]), compare);



	


	// ANS_DIR folder;
	DIR *dir2 = opendir(argv[2]);
	if (dir2 == NULL) {
		printf("failed open for answer file\n");
		exit(1);
	}
	struct dirent *de2 = NULL;

	//ANS_DIR folder name save
	while ((de2 = readdir(dir2)) != NULL) {

		//save
		if ((!strcmp(de2->d_name, ".")) || (!strcmp(de2->d_name, "..")))
			continue;

		answerQ[num_a] = de2->d_name;
		num_a++;
		//num_a : total answer file number
	}

	for (j = 0; j < num_a; j++) {
		printf("%s\n", answerQ[j]);
	}

	for (j = 0; j < num_s; j++) {
		printf("%s\n", studentId[j]);
	}
	printf("***********************************************\n");


	//----------------------------------------------------------------------
	DIR *subdir = NULL;

	//for directory, root setting
	char path[MAX_SIZE];
	char subpath[MAX_SIZE];
	getcwd(path, 200);
	strcat(path, "/");
	strcat(path, argv[1]);
	strcat(path, "/");
	strcpy(subpath, path);

	char path_a[MAX_SIZE];
	getcwd(path_a, 200);
	strcat(path_a, "/");
	strcat(path_a, argv[2]);
	strcat(path_a, "/");

	int k,m,n;
	struct ans_num*num;
	char* text_f;
	num = (struct ans_num*)malloc(sizeof(struct ans_num)* MAX_SIZE);
	//loop for student directory
	for(i=0;i<num_a;i++){
	for (j = 0; j < num_s; j++) {
		printf("---------%s : %s-----\n", answerQ[i], studentId[j]);
		strcat(path, studentId[j]);
		//return is struct => filename, filetype 
		subdirOutput(path_a, answerQ[j], path, studentId[j]);
	strcpy(path, subpath);
	


	//???------------------------------------------------------------???//
	for (m = 0; m < innerFileCount; m++) {
		printf("%s\n", studentFile[m]);
//	}


//	for (m = 0; m < innerFileCount; m++) {
//		printf("\\\\\\\%s\n",studentFile[m]);
		num[m].name = studentFile[m];
		num[m].num = m;
		char* test_l = (char*)malloc(1000);
		strcpy(test_l, studentFile[m]);
		text_f = strtok(test_l, ".-");
		while (text_f != NULL) {
			text_f = strtok(NULL, ".");
			if (strcmp(text_f, "txt") == 0) {
				num[m].type = "txt";
				break;
			}
			else if (strcmp(text_f, "c") == 0) {
				num[m].name_s = 0;
				num[m].type = "c";
				break;
			}
			num[m].name_s = atoi(text_f);
		}
		num[m].name_i = atoi(studentFile[m]);
	}
	//studentFile name sort
	
	ans_numSort(num);
	for (n = 0; n < innerFileCount; n++) {
		studentFile[n] = num[n].name;
	}
	for (k = 0; k < innerFileCount; k++) {
		printf("%s\n", studentFile[k]);
	}

	//-------------------
	


	}
	}
	
	
	closedir(dir);
	closedir(dir2);

//	gettimeofday(&end_t, NULL);
//	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
void subdirOutput(char*wd_a, char*answerQ, char*wd, char*studentId) {

	 innerFileCount = 0;
	

	struct dirent*dentry;
	DIR*dirp;
	if (chdir(wd_a) < 0 || chdir(wd) < 0) {
		printf("error:chdir../n");
		exit(1);
	}

	if ((dirp = opendir(".")) == NULL) {
		printf("error:opendir..\n");
		exit(1);
	}

	while (dentry = readdir(dirp)) {
		if (innerFileCount >= 100) {
			printf("cannotsave\n");
		}
		if ((!strcmp(dentry->d_name, ".")) || (!strcmp(dentry->d_name, "..")))
			continue;
		studentFile[innerFileCount] = dentry->d_name;
		innerFileCount++;

	}



/*
	int j;
	for (j = 0; j < innerFileCount; j++) {
		printf("%s\n", studentFile[j]);
	}
*/

	

	closedir(dirp);

}
void ans_numSort(struct ans_num* num) {
	//sort
	int i, j;
	struct ans_num num2;
	for (i = 0; i < innerFileCount; i++) {
		for (j = 0; j < i; j++) {
			if (num[i].name_i < num[j].name_i) {
				num2 = num[i];
				num[i] = num[j];
				num[j] = num2;

			}
			if (num[i].name_i == num[j].name_i) {
				if (num[i].name_s < num[j].name_s) {
					num2 = num[i];
					num[i] = num[j];
					num[j] = num2;

				}
			}
		}
	}

}
