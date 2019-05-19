#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <pthread.h>
#include "ssu_runtime.h"

#define MAX_SIZE 1024
#define TIME_LIMIT 5
#define WARNING_SCORE -0.1
#define ERROR_SCORE 0

struct ans_num {
	char name[MAX_SIZE];
	char file_name[MAX_SIZE];
	int name_i;
	int name_s; 
	int num;
	double score;
};

double finalScore[100][100] = { 0 };
double sum[100] = { 0 };

int compare(const void*a, const void*b) {
	return strcmp(*(char**)a, *(char**)b);
}
void makeScore_table(char*, struct ans_num*, int);
void makeFinal_Score_table(char**stdId, struct ans_num* num, char* folder, int stdCount, int ansCount);
void *threadRoutine(void *argumentPointer);
void gradingC(char*problemC, char*path_a);
static int check_thread_status(char *pnWorkStatus, int nWaitTime);
int list_dir(const char* path,char**saveName);
void ans_numSort(struct ans_num* num, int);
void subdirOutput(char *wd, char* studentId, char*wd_a, char*answerQ);
void programCompile(char*answerQ,char*path,int ch ,char*stdId);
char* studentFile[MAX_SIZE];
void EraseSpace(char *ap_string);
int innerFileCount;
int e_flag = 0;
int p_flag = 0;
int t_flag = 0;
int c_flag = 0;
void optionE(char*);
void optionP();
void optionH();
void optionC(int count);
char cName[5][MAX_SIZE];
char eName[MAX_SIZE];
char tName[5][MAX_SIZE];
int optCount1 = 0;
int optCount2 = 0;

int main(int argc, char **argv)
{
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);
	char* studentId[MAX_SIZE];
	char* answerQ[MAX_SIZE];

	if (argc == 1) {
		fprintf(stderr, "Usage : ssu_score <STUDENTDIR> <TRUEDIR> [OPTION]\n");
		exit(1);
	}

	if (!strcmp(argv[1], "-h")) {
		optionH();
		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		exit(0);
	}

	if (!strcmp(argv[1], "-c")) {
		//미구현
	}

	char std_folder[MAX_SIZE];
	char ans_folder[MAX_SIZE];

	strcpy(std_folder, argv[1]);
	strcpy(ans_folder, argv[2]);

	int array_count = 0;

	while (array_count < argc) {

		if (argv[array_count][0] == '-') {

			switch (argv[array_count][1]) {
				case 'h':
					optionH();
					gettimeofday(&end_t, NULL);
					ssu_runtime(&begin_t, &end_t);
					exit(0);
				case 'p':
					array_count++;
					p_flag = 1;
					optionP();
					break;
				case 'e':
					if ((array_count + 1) != argc) {
						if (argv[array_count + 1][0] != '-') {
							e_flag = 1;
							strcpy(eName, argv[array_count + 1]);
							array_count++;
						}
						else {
							printf("Usage : -e <DIRNAME>\n");
							exit(1);
						}
					}
					else {
						printf("Usage : -e <DIRNAME>\n");
						exit(1);
					}
					break;
				case't':
					if ((array_count + 1) != argc) {
						if (argv[array_count + 1][0] == '-') {
							array_count++;
							printf("Usage : -t <QNAME1> <QNAME2> ... <QNAME5>\n");
							break;
						}
						else {
							t_flag = 1;
							while (optCount2 < 5) {
								array_count++;
								strcpy(tName[optCount2], argv[array_count]);
								optCount2++;
								if (array_count + 1 >= argc) {
									break;
								}

							}
						}
					}
					else {
						printf("-t's variable argument is not detected!");
						printf("Usage : -t <QNAME1> <QNAME2> ... <QNAME5>\n");
						array_count++;
						break;
					}
					break;
				case'c':
					if ((array_count + 1) != argc) {
						if (argv[array_count + 1][0] == '-') {
							array_count++;
							printf("Usage : -c <ID1> <ID2> ... <ID5>\n");
							break;
						}
						else {
							c_flag = 1;
							while (optCount1 < 5) {
								strcmp(tName[optCount2], argv[array_count]);
								optCount1++;
								array_count++;
								if (argv[array_count + 1][0] == '-') {
									break;
								}
							}
						}
					}
					else {
						array_count++;
						printf("Usage : -c <ID1> <ID2> ... <ID5>\n");
						break;
					}
					break;
				default:
					printf("Unknown option %c\n", optopt);
					exit(1);
			}
		}
		else
			array_count++;
	}

	DIR *dir = opendir(std_folder);
	if (dir == NULL) {
		printf("first directory failed open\n");
		exit(1);
	}

	struct dirent *de = NULL;
	struct dirent *subde = NULL;
	int i, j;
	int num_s = 0;
	int num_a = 0;

	while ((de = readdir(dir)) != NULL) {
		if (num_s >= 100) {
			printf("can not save\n");
		}
		if ((!strcmp(de->d_name, ".")) || (!strcmp(de->d_name, "..")))
			continue;
		studentId[num_s] = de->d_name;
		num_s++;
	}
	qsort(studentId, num_s, sizeof(studentId[0]), compare);

	DIR *dir2 = opendir(ans_folder);
	if (dir2 == NULL) {
		printf("failed open for answer file\n");
		exit(1);
	}
	struct dirent *de2 = NULL;
	struct dirent **namelist;
	char * pathName;
	char srcName[PATH_MAX];
	char tarName[PATH_MAX];
	int listcount;
	char path[MAX_SIZE];
	int problemC = 0;
	char *proNum[MAX_SIZE];
	if ((listcount = scandir(ans_folder, &namelist, NULL, alphasort)) == -1) {
		fprintf(stderr, "%s Directory Scan Err\n", ans_folder);
		exit(1);
	}

	for (i = 2; i < listcount; i++) {
		if (strstr(namelist[i]->d_name, ".csv") != NULL) {
			listcount--;
			continue;
		}
		answerQ[i - 2] = namelist[i]->d_name;
	}

	struct ans_num* num;
	num = (struct ans_num*)malloc(sizeof(struct ans_num)* MAX_SIZE);

	char* text_f;
	int dcheck = 0;
	for (i = 0; i < listcount - 2; i++) {
		strcpy(num[i].name, answerQ[i]);
		num[i].num = i;
		char* text_l = (char*)malloc(1000);
		strcpy(text_l, answerQ[i]);
		text_f = strtok(text_l, "-");
		while (text_f != NULL) {
			dcheck = 1;
			num[i].name_s = atoi(text_f);
			text_f = strtok(NULL, " ");
		}
		if (!strstr(num[i].name, "-")) {
			num[i].name_s = 0;
		}
		num[i].name_i = atoi(answerQ[i]);
		free(text_l);
	}

	ans_numSort(num, listcount - 2);

	for (i = 0; i < listcount - 2; i++) {
		strcpy(answerQ[i], num[i].name);
	}

	for (i = 0; i < listcount - 2; i++) {
		if (num[i].name_s != 0) {
			sprintf(num[i].file_name, "%s.txt", num[i].name);
		}
		else {
			sprintf(num[i].file_name, "%s.c", num[i].name);
		}
	}

	makeScore_table(ans_folder, num, listcount);

	getcwd(path, 200);
	char* proNum_txt[MAX_SIZE];
	int problemT = 0;
	char nPath[PATH_MAX];
	strcpy(nPath, path);
	for (i = 2; i < listcount; i++) {
		char* problemNum = namelist[i]->d_name;
		sprintf(nPath, "%s/%s/%s", path, ans_folder, namelist[i]->d_name);
		if (strchr(answerQ[i - 2], '-') == NULL) {
			proNum[problemC] = namelist[i]->d_name;
			problemC++;
			programCompile(nPath, problemNum, 0, NULL);
		}
		else {
			proNum_txt[problemT] = namelist[i]->d_name;
			problemT++;
		}
	}

	struct stat file_info;
	getcwd(path, 200);
	struct dirent **nlist;
	char*saveName[PATH_MAX];
	int k;
	char path1[MAX_SIZE];
	int new_listcount;

	if (e_flag) {
		char dirPathName[MAX_SIZE];
		sprintf(dirPathName, "%s/%s", path, eName);

		if (mkdir(dirPathName, 0777) < 0) {
			char deleteDirname[MAX_SIZE];
			sprintf(deleteDirname, "rm -rf %s", dirPathName);
			system(deleteDirname);
			if (mkdir(dirPathName, 0777) < 0) {
				fprintf(stderr, "mkdir error");
				exit(1);
			}
		}

		char studentPathName[MAX_SIZE];
		int p;
		for (p = 0; p < num_s; p++) {
			sprintf(studentPathName, "%s/%s", eName, studentId[p]);
			if (mkdir(studentPathName, 0777) < 0) {
				fprintf(stderr, "mkdir error for %s", studentPathName);
				exit(1);
			}
		}
	}

	for (j = 0; j < num_s; j++) {
		sprintf(path1, "%s/%s/%s", path, std_folder, studentId[j]);
		int ffd;
		new_listcount = list_dir(path1, saveName);
		struct ans_num*num_sf;
		num_sf = (struct ans_num*)malloc(sizeof(struct ans_num)* new_listcount);
		char* text_f2;

		for (i = 0; i < new_listcount; i++) {
			strcpy(num_sf[i].name, saveName[i]);
			num_sf[i].num = i;
			char* test_l2 = (char*)malloc(1000);
			strcpy(test_l2, saveName[i]);
			text_f2 = strtok(test_l2, ".-");
			while (text_f2 != NULL) {
				text_f2 = strtok(NULL, ".");
				num_sf[i].name_s = atoi(text_f2);
				if (strcmp(text_f2, "txt") == 0) {
					break;
				}
				else if (strcmp(text_f2, "c") == 0) {
					num_sf[i].name_s = 0;
					break;
				}
				else
					break;
			}
			num_sf[i].name_i = atoi(saveName[i]);
		}
		ans_numSort(num_sf, new_listcount);

		for (i = 0; i < new_listcount; i++) {
			saveName[i] = num_sf[i].name;
			int newfd;
			for (i = 0; i < new_listcount; i++) {
				if (strchr(saveName[i], '-') == NULL) {
					if (strstr(saveName[i], ".c") != NULL) {
						char pronum[MAX_SIZE];
						strcpy(pronum, strtok(saveName[i], "."));
						programCompile(path1, pronum, 1, studentId[j]);

					}
				}
			}
		}

		FILE *noSpace_ans_txt;
		FILE *noSpace_std_txt;
		FILE *ans_txt;
		FILE *std_txt;
		char txt_path_ans[10000];
		char txt_path_std[10000];

		int s1, s2;
		for (i = 0; i < problemT; i++) {
			sprintf(txt_path_ans, "%s/%s/%s/%s.txt", path, ans_folder, proNum_txt[i], proNum_txt[i]);
			ans_txt = fopen(txt_path_ans, "r");
			char std_txt_context[3000] = "";
			char ans_txt_context[MAX_SIZE] = "";
			char *answer_context = NULL;
			char *stud_context = NULL;
			answer_context = fgets(ans_txt_context, sizeof(ans_txt_context), ans_txt);

			for (j = 0; j < num_s; j++) {
				sprintf(txt_path_std, "%s/%s/%s/%s.txt", path, std_folder, studentId[j], proNum_txt[i]);
				std_txt = fopen(txt_path_std, "r");
				int fd_std_txtFile = 0;
				int fd_std_txtFile_size = 0;
				fd_std_txtFile = open(txt_path_std, O_RDWR, 0777);
				fd_std_txtFile_size = lseek(fd_std_txtFile, 0, SEEK_END);
				if (fd_std_txtFile_size <= 1) {
					close(fd_std_txtFile);
					continue;
				}
				close(fd_std_txtFile);
				if (std_txt == NULL)
					continue;
				int tokenNumber = 0;
				stud_context = fgets(std_txt_context, sizeof(std_txt_context), std_txt);
				if (stud_context == NULL)
					continue;
				char* txt_f = (char*)malloc(1000);
				char* txt_l = (char*)malloc(1000);
				strcpy(txt_l, answer_context);

				char txt_s_1[100];
				strcpy(txt_s_1, stud_context);
				char buf_std[100][100];
				char buf[100][100];
				int tokenNumber_std = 0;
				strcpy(txt_s_1, strtok(txt_s_1, " \n"));
				strcpy(buf_std[tokenNumber_std], txt_s_1);
				while (txt_s_1 != NULL) {
					char *txt_s_1_1 = NULL;
					txt_s_1_1 = strtok(NULL, " \n");
					if (txt_s_1_1 == NULL) break;
					tokenNumber_std++;
					strcpy(buf_std[tokenNumber_std], txt_s_1_1);
				}

				while (1) {
					tokenNumber = 0;
					char text_second[MAX_SIZE];
					txt_f = strtok(txt_l, ":");

					if (txt_l == NULL) {
						strcpy(text_second, txt_f);
					}
					else {
						strcpy(txt_l, txt_f);
					}
					char txt_f_2[100];
					strcpy(txt_f_2, strtok(txt_f, " \n"));
					if (txt_f == NULL) {
						strcpy(buf[tokenNumber], txt_f_2);
						strcpy(txt_f, txt_f_2);
					}
					else {
						strcpy(buf[tokenNumber], txt_f);
					}
					while (txt_f != NULL) {
						txt_f = strtok(NULL, " \n");
						if (txt_f == NULL) break;
						tokenNumber++;
						strcpy(buf[tokenNumber], txt_f);
					}

					int token_check = 0;
					if (tokenNumber_std == tokenNumber) {
						int tok;
						for (tok = 0; tok < tokenNumber_std; tok++) {
							token_check = 3;
							if (!strcmp(buf_std[tok], buf[tok])) {
								token_check = 1;
								continue;
							}
							if (token_check == 3) {
								break;
							}
						}
						if (token_check == 1) {
							int t_ccount;
							int t_ind_num = 0;
							for (t_ccount = 0; t_ccount < listcount - 2; t_ccount++) {
								if (!strcmp(answerQ[t_ccount], proNum_txt[i])) {
									t_ind_num = t_ccount;
									break;
								}
							}
							int ss_id = atoi(studentId[j]) - 20190001;
							finalScore[ss_id][t_ind_num] = num[t_ccount].score;

						}
						else {
							txt_l = strtok(text_second, ":");
							if (txt_l == NULL) {
								break;
							}
							else {
								tokenNumber = 0;
							}
						}
					}
					else {
						break;
					}
					break;
				}
				s2 = fclose(std_txt);
				if (s2 != 0) {
					printf("stream close error\n");

				}
				free(txt_f);
				free(txt_l);
			}
			s1 = fclose(ans_txt);
			if (s1 != 0) {
				printf("stream close error\n");
			}
		}

		FILE *answerFile;
		FILE *noSpaceAnsFile;
		FILE *stdFile;
		FILE *noSpaceStdFile;
		char path_ans[MAX_SIZE];
		char no_path_a[MAX_SIZE];

		for (i = 0; i < problemC; i++) {
			sprintf(path_ans, "%s/%s/%s", path, ans_folder, proNum[i]);
			sprintf(no_path_a, "%s/%s/%s/no_spa_%s.txt", path, ans_folder, proNum[i], proNum[i]);

			gradingC(proNum[i], path_ans);

			for (j = 0; j < num_s; j++) {
				char path_std[MAX_SIZE];
				char no_path_s[MAX_SIZE];

				noSpaceAnsFile = fopen(no_path_a, "r");

				sprintf(path_std, "%s/%s/%s", path, std_folder, studentId[j]);
				sprintf(no_path_s, "%s/%s/%s/no_spa_%s.txt", path, std_folder, studentId[j], proNum[i]);

				gradingC(proNum[i], path_std);
				noSpaceStdFile = fopen(no_path_s, "r");

				int fd_stdFile_size = 0;
				int fd_ansFile_size = 0;
				int fd_stdFile, fd_ansFile;
				fd_stdFile = open(no_path_a, O_RDWR, 0777);
				fd_ansFile = open(no_path_s, O_RDWR, 0777);
				fd_stdFile_size = lseek(fd_stdFile, 0, SEEK_END);
				fd_ansFile_size = lseek(fd_ansFile, 0, SEEK_END);

				if (fd_stdFile_size == 0 || fd_ansFile_size == 0)
					continue;
				close(fd_stdFile);
				close(fd_ansFile);

				int state1, state2;
				while (1) {
					char strTemp[MAX_SIZE] = "";
					char ansTemp[MAX_SIZE] = "";
					char *pAnswerLine = NULL;
					char *pStdLine = NULL;

					pAnswerLine = fgets(ansTemp, sizeof(ansTemp), noSpaceAnsFile);
					pStdLine = fgets(strTemp, sizeof(strTemp), noSpaceStdFile);

					if (pAnswerLine == NULL && pStdLine == NULL) {
						int r;
						int index_numlist = 0;
						for (r = 0; r < listcount - 2; r++) {
							if (!strcmp(answerQ[r], proNum[i])) {
								index_numlist = r;
								break;
							}
						}
						int stud_Id = atoi(studentId[j]) - 20190001;
						finalScore[stud_Id][index_numlist] = num[r].score;
						break;
					}
					else if (pAnswerLine == NULL || pStdLine == NULL) {
						break;
					}
					if (!strcasecmp(pAnswerLine, pStdLine)) {
						continue;
					}
					else {
						break;
					}
				}
				state1 = fclose(noSpaceAnsFile);
				state2 = fclose(noSpaceStdFile);
				if (state1 != 0 || state2 != 0) {
					printf("stream close error\n");

				}
			}
		}

		for (i = 0; i < num_s; i++) {
			for (j = 0; j < listcount - 2; j++) {
				sum[i] += finalScore[i][j];
			}
		}

		if (!e_flag) {

			for (i = 0; i < problemC; i++) {
				char deleteAns[MAX_SIZE];
				sprintf(deleteAns, "%s/%s/%s/%s_err.txt", path, ans_folder, proNum[i], proNum[i]);

				for (j = 0; j < num_s; j++) {
					char deleteStd[MAX_SIZE];
					sprintf(deleteStd, "%s/%s/%s/%s_err.txt", path, std_folder, studentId[j], proNum[i]);
					char errCheckName[MAX_SIZE];
					FILE* errCheckf;
					errCheckf = fopen(deleteStd, "r");

					while (1) {
						char errAnsTemp[MAX_SIZE] = "";
						char *perrAnswerLine = NULL;
						perrAnswerLine = fgets(errAnsTemp, sizeof(errAnsTemp), errCheckf);

						int er;
						int erindex_numlist = 0;
						for (er = 0; er < listcount - 2; er++) {
							if (!strcmp(answerQ[er], proNum[i])) {
								erindex_numlist = er;
								break;
							}
						}
						if (strstr(perrAnswerLine, "warning") != NULL) {
							int erstud_Id = atoi(studentId[j]) - 20190001;
							if (num[er].score <= 0) {
								num[er].score = 0;
								break;
							}
							finalScore[erstud_Id][erindex_numlist] = num[er].score + WARNING_SCORE;
							continue;
						}
						else if (strstr(perrAnswerLine, "error") != NULL) {
							int erstud_Id = atoi(studentId[j]) - 20190001;
							finalScore[erstud_Id][erindex_numlist] = ERROR_SCORE;
							continue;
						}

						if (perrAnswerLine == NULL) {
							break;
						}

					}
					fclose(errCheckf);

					remove(deleteStd);
				}
				remove(deleteAns);
			}
		}
		else {
			char deleteAns[MAX_SIZE];
			sprintf(deleteAns, "%s/%s/%s/%s_err.txt", path, ans_folder, proNum[i], proNum[i]);
			remove(deleteAns);

			for (j = 0; j < num_s; j++) {
				int allnoerror = 0;
				for (i = 0; i < problemC; i++) {
					char deleteStd[MAX_SIZE];
					int fd_error_txt;
					int fd_error_size;
					sprintf(deleteStd, "%s/%s/%s/%s_err.txt", path, eName, studentId[j], proNum[i]);
					char errorCheckName[MAX_SIZE];
					FILE* errorCheckf;
					errorCheckf = fopen(deleteStd, "r");

					while (1) {
						char errorAnsTemp[MAX_SIZE] = "";
						char *perrorAnswerLine = NULL;
						perrorAnswerLine = fgets(errorAnsTemp, sizeof(errorAnsTemp), errorCheckf);

						int eror;
						int erorindex_numlist = 0;
						for (eror = 0; eror < listcount - 2; eror++) {
							if (!strcmp(answerQ[eror], proNum[i])) {
								erorindex_numlist = eror;
								break;
							}
						}
						if (strstr(perrorAnswerLine, "warning") != NULL) {
							int erorstud_Id = atoi(studentId[j]) - 20190001;
							if (num[eror].score <= 0) {
								num[eror].score = 0;
								break;
							}
							finalScore[erorstud_Id][erorindex_numlist] = num[eror].score + WARNING_SCORE;
							continue;
						}
						else if (strstr(perrorAnswerLine, "error") != NULL) {
							int erorstud_Id = atoi(studentId[j]) - 20190001;
							finalScore[erorstud_Id][erorindex_numlist] = ERROR_SCORE;
							continue;
						}

						if (perrorAnswerLine == NULL) {
							break;
						}

					}

					fclose(errorCheckf);

					fd_error_txt = open(deleteStd, O_RDWR, 0777);
					fd_error_size = lseek(fd_error_txt, 0, SEEK_END);
					close(fd_error_txt);
					if (fd_error_size == 0) {
						allnoerror++;
						remove(deleteStd);
					}
				}
				if (allnoerror == problemC) {
					char deleteStdFile[MAX_SIZE];
					sprintf(deleteStdFile, "%s/%s/%s", path, eName, studentId[j]);
					rmdir(deleteStdFile);
				}
			}
		}

		makeFinal_Score_table(studentId, num, ans_folder, num_s, listcount - 2);

		for (i = 0; i < problemC; i++) {
			char delete_no_spa_Ans[MAX_SIZE];
			sprintf(delete_no_spa_Ans, "%s/%s/%s/no_spa_%s.txt", path, ans_folder, proNum[i], proNum[i]);

			for (j = 0; j < num_s; j++) {
				char delete_no_spa_Std[MAX_SIZE];
				sprintf(delete_no_spa_Std, "%s/%s/%s/no_spa_%s.txt", path, std_folder, studentId[j], proNum[i]);
				remove(delete_no_spa_Std);
			}
			remove(delete_no_spa_Ans);
		}

		double average = 0;
		int u_count;
		if (p_flag) {
			for (u_count = 0; u_count < num_s; u_count++) {
				printf("%s is finished.. score : %.1lf\n", studentId[u_count], sum[u_count]);
				average += sum[u_count];
			}
			printf("Total average : %.2lf\n", average / num_s);
		}
		else {
			for (u_count = 0; u_count < num_s; u_count++) {
				printf("%s is finished..\n", studentId[u_count]);
			}

		}

		if (c_flag) {
			optionC(optCount1);
		}
		closedir(dir);
		closedir(dir2);

		free(num);

		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		exit(0);

	}
}
void makeScore_table(char* folder, struct ans_num* num, int listcount) {
	FILE *pFile;
	char pFilePath[MAX_SIZE];
	sprintf(pFilePath, "./%s/score_table.csv", folder);
	int selectScore;
	double b_score;
	double p_score;
	double eachScore;
	int i;

	if ((pFile = fopen(pFilePath, "r+")) == NULL) {
		printf("score_table.csv file doesn't exist in %s!\n", folder);
		pFile = fopen(pFilePath, "w+");
		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &selectScore);

		switch (selectScore) {
			case 1:
				printf("Input value of blank question : ");
				scanf("%lf", &b_score);
				printf("Input value of program question : ");
				scanf("%lf", &p_score);

				for (i = 0; i < listcount - 2; i++) {
					if (num[i].name_s != 0) {
						fprintf(pFile, "%s,%.2lf\n", num[i].file_name, b_score);
						num[i].score = b_score;
					}
					else {
						fprintf(pFile, "%s,%.2lf\n", num[i].file_name, p_score);
						num[i].score = p_score;
					}
				}
				break;
			case 2:
				for (i = 0; i < listcount - 2; i++) {
					printf("Input of %s: ", num[i].file_name);
					scanf("%lf", &eachScore);
					fprintf(pFile, "%s,%.2lf\n", num[i].file_name, eachScore);
					num[i].score = eachScore;
				}
				break;
			default:
				printf("wrong type!\n");
				exit(1);
		}
	}
	else {
		i = 0;
		char a[1024];
		char b[1024];
		while(fgets(a,sizeof(a),pFile)!=NULL){
			strcpy(b,strtok(a,","));
			strcpy(num[i].file_name, b);;
			strcpy(b,strtok(NULL,",a"));
			num[i].score=atof(b);
			i++;
			if(strtok(NULL,",")=="\n")
				continue;
		}
	}
	fclose(pFile);
}

void makeFinal_Score_table(char** stdId, struct ans_num* num, char* folder, int stdCount, int ansCount) {
	int row, col;
	FILE *pFile;
	char pFilePath[MAX_SIZE];
	strcpy(pFilePath, "./score.csv");
	int i,j;

	pFile = fopen(pFilePath, "w+");	
	fprintf(pFile, " ,");

	for (i = 0; i < ansCount; i++) {
		fprintf(pFile, "%s,", num[i].file_name);
	}

	fprintf(pFile, "sum\n");

	for (i = 0; i < stdCount; i++) {
		fprintf(pFile, "%d,",atoi(stdId[i]));
		for (j = 0; j < ansCount; j++) {
			fprintf(pFile, "%.2lf,", finalScore[i][j]);
		}
		fprintf(pFile, "%.2lf\n", sum[i]);
	}
	fclose(pFile);
}

void EraseSpace(char *ap_string){
	char *p_dest = ap_string;
	while (*ap_string != 0) {

		if (*ap_string != ' ') {
			if (p_dest != ap_string) *p_dest = *ap_string;
			p_dest++;
		}
		ap_string++;
	}
	*p_dest = 0;
}

int list_dir(const char* path,char** saveName){
	struct dirent *entry;
	DIR *dir = opendir(path);
	int count=0;
	if(dir==NULL){
		return count;
	}
	while((entry = readdir(dir))!=NULL){
		if ((!strcmp(entry->d_name, ".")) || (!strcmp(entry->d_name, "..")))
			continue;
		saveName[count]=entry->d_name;
		count++;
	}	
	closedir(dir);
	return count;
}

void gradingC( char*problemC, char*path) {
	FILE  *noSpaceAnsFile;
	FILE *answerFile;

	char no_path_a[MAX_SIZE];
	char path_a[MAX_SIZE];
	sprintf(path_a,"%s/%s.stdout",path,problemC);
	sprintf(no_path_a, "%s/no_spa_%s.txt", path,problemC);

	noSpaceAnsFile = fopen(no_path_a, "w+");
	answerFile = fopen(path_a, "r+"); 

	if (answerFile != NULL)	{
		char strTemp[MAX_SIZE];
		char* pStr=NULL;
		char* pStr_backup=NULL;

		do{
			pStr = fgets(strTemp, sizeof(strTemp), answerFile);
			if(feof(answerFile)) break;
			pStr_backup=pStr;
			EraseSpace(pStr);
			fprintf(noSpaceAnsFile,"%s", pStr);
		}
		while (!feof(answerFile));
		fclose(noSpaceAnsFile);
		fclose(answerFile);
	}

}
void programCompile(char* path,char* problemNum, int anscheck ,char*stdId){
	char* gccname="gcc -o";
	char name[PATH_MAX];
	int i;
	pthread_t threadID;
	char checkname[PATH_MAX];
	sprintf(checkname,"%s/%s.exe",path,problemNum);
	sprintf(name,"%s %s %s/%s.c",gccname,checkname,path,problemNum);
	int check;
	int fd1,fd2,bk,bk2;

	bk=open("dummy",O_WRONLY|O_CREAT);
	bk2=open("dummy2",O_WRONLY|O_CREAT);
	close(bk);
	close(bk2);
	dup2(1,bk);
	dup2(2,bk2);
	char res[PATH_MAX];
	char errRedirection[PATH_MAX];

	sprintf(res,"%s/%s.stdout",path,problemNum);

	if((fd2=open(res,O_WRONLY|O_CREAT|O_TRUNC))==-1){
		fprintf(stderr, "open error for %s\n",res);
		exit(1);
	}
	fchmod(fd2,0777);
	dup2(fd2,1);

	if(e_flag&&anscheck){
		char e_errRedirection[MAX_SIZE];
		char now_path[MAX_SIZE];
		getcwd(now_path,200);
		sprintf(e_errRedirection,"%s/%s/%s/%s_err.txt",now_path,eName,stdId,problemNum);
		if((fd1=open(e_errRedirection,O_WRONLY|O_CREAT|O_TRUNC))==-1){
			fprintf(stderr, "open error for %s\n",e_errRedirection);
			exit(1);
		}
		fchmod(fd1,0777);
		dup2(fd1,2);
	}
	else{
		sprintf(errRedirection,"%s/%s_err.txt",path,problemNum);
		if((fd1=open(errRedirection,O_WRONLY|O_CREAT|O_TRUNC))==-1){
			fprintf(stderr, "open error for %s\n",errRedirection);
			exit(1);
		}
		fchmod(fd1,0777);
		dup2(fd1,2);
	}

	system(name);

	char argument[MAX_SIZE];
	sprintf(argument, "%s", checkname);
	int ch=0;
	char e_errRedirection[MAX_SIZE];

	if((check=access(checkname,F_OK))<0){
		if(t_flag){
			int h;
			dup2(bk,1);
			dup2(bk2,2);

			close(fd2);
			close(fd1);

			if((fd2=open(res,O_WRONLY|O_CREAT|O_TRUNC))==-1){
				fprintf(stderr, "open error for %s\n",res);
				exit(1);
			}
			fchmod(fd2,0777);
			dup2(fd2,1);
			if(e_flag&&anscheck){
				char now_path[MAX_SIZE];
				getcwd(now_path,200);
				sprintf(e_errRedirection,"%s/%s/%s/%s_err.txt",now_path,eName,stdId,problemNum);
				if((fd1=open(e_errRedirection,O_WRONLY|O_CREAT|O_TRUNC))==-1){
					fprintf(stderr, "open error for %s\n",e_errRedirection);
					exit(1);
				}
				fchmod(fd1,0777);
				dup2(fd1,2);
			}
			else{
				sprintf(errRedirection,"%s/%s_err.txt",path,problemNum);
				if((fd1=open(errRedirection,O_WRONLY|O_CREAT|O_TRUNC))==-1){
					fprintf(stderr, "open error for %s\n",errRedirection);
					exit(1);
				}
				fchmod(fd1,0777);
				dup2(fd1,2);
			}

			for(h=0;h<optCount2;h++){
				if(!strcmp(tName[h],problemNum)){
					char new_system_name[MAX_SIZE];
					sprintf(new_system_name,"%s -lpthread",name);
					system(new_system_name);
				}
			}
		}
		else{
			ch=1;
			dup2(bk,1);
			dup2(bk2,2);
			remove("dummy");
			remove("dummy2");
			close(fd2);
			close(fd1);
		}
	}

	if(ch==0){
		pthread_create(&threadID, NULL, threadRoutine, (void*)argument);
		pthread_detach(threadID);
		if (!check_thread_status(argument, TIME_LIMIT)) {
			pthread_cancel(threadID);
		}
		dup2(bk,1);
		dup2(bk2,2);
		remove("dummy");
		remove("dummy2");
		close(fd2);
		close(fd1);
	}

	if(e_flag){
		off_t filesize;
		filesize = lseek(fd1,0,SEEK_END);
		if(filesize==0){

			remove(e_errRedirection);
		}
	}

}

void *threadRoutine(void *argumentPointer){
	char *argument = (char *)argumentPointer;
	system(argument);
	strcpy(argument, "end");
	return NULL;
}

static int check_thread_status(char *pnWorkStatus, int nWaitTime){
	int i;
	for (i = 0; i < nWaitTime; i++){
		if (!strcmp(pnWorkStatus, "end"))
			return 1;
		sleep(1);
	}
	return 0;
}

void optionE(char* eName) {
	char err[MAX_SIZE];
	sprintf(err, "./%s", eName);
	DIR *err_directory = NULL;
	int res;

	res = access(err, 0);
	if (res == -1) {
		mkdir(err, 0777);
	}
}

void optionP() {
	//미구현
}

void optionH() {
	printf("Usage : ssu_score <STUDENTDIR> <TRUEDIR> [OPTION]\n");
	printf("Option : \n");
	printf(" -e <DIRNAME>	print error on 'DIRNAME/ID/qname_error.txt' file\n");
	printf(" -t <QNAMES>	compile QNAME.C with -lpthread option\n");
	printf(" -h		print usage\n");
	printf(" -p		print student's score and total average\n");
	printf(" -c <IDS> 	print ID's score\n");
}

void optionC(int count) {

	struct dirent *dentry;
	struct stat fstat;
	DIR *dirp;
	int i;
	int check;
	char path[MAX_SIZE];

	if ((dirp = opendir(".")) == NULL) {
		printf("error: opendir..\n");
		exit(1);
	}

	while (dentry = readdir(dirp)) {
		if (dentry->d_ino != 0) {
			if ((!strcmp(dentry->d_name, ".")) || (!strcmp(dentry->d_name, "..")))
				continue;
			stat(dentry->d_name, &fstat);
			if (S_ISDIR(fstat.st_mode)) {
				chdir(dentry->d_name);
			}
			else if(S_ISREG(fstat.st_mode)){
				if (!strcmp(dentry->d_name, "score.csv")) {
					strcpy(path, dentry->d_name);
					check = 1;
					break;
				}

			}
		}
	}

	FILE *pFile;
	char str_tmp[MAX_SIZE];
	int cnt ;
	int total = 0;
	int token_count=0;
	char *total_sum[100];
	char* p;
	char *b[MAX_SIZE];
	if ((pFile = fopen(path, "r+")) != NULL) {
		fgets(str_tmp, MAX_SIZE, pFile);
		cnt = 0;
		p = strtok(str_tmp, ",");
		while (p != NULL) {
			sprintf(b[cnt], "%s", p);
			cnt++;
			p = strtok(NULL, ",");
		}

		while (!feof(pFile)) {
			fgets(str_tmp, MAX_SIZE, pFile);
			p = strtok(str_tmp, ",");
			while(p!=NULL){
				for (i = 0; i < cnt; i++) {
					p = strtok(NULL, ",");
				}
				sprintf(total_sum[total], "%s", p);
				token_count++;
			}
		}
		for (i = 0; i < count; i++) {
			printf("%s's score : %s", cName[i], total_sum[atoi(cName[i]) - 20190000]);
		}

		if(check==0){
			printf("file is not exist!\n");
		}

		fclose(pFile);
	}
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
	closedir(dirp);
}
void ans_numSort(struct ans_num* num, int cnt) {
	int i, j;
	struct ans_num num2;
	for (i = 0; i < cnt; i++) {
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
