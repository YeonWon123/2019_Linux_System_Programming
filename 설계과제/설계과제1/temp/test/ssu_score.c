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

//total question number
struct ans_num {
	char name[MAX_SIZE];
	char file_name[MAX_SIZE];
	int name_i;
	int name_s; // ==0 -> .c, !=0 -> .txt
	int num;
	double score;
};

double finalScore[100][100] = { 0 };
double sum[100] = { 0 };

int compare(const void*a, const void*b) {
	return strcmp(*(char**)a, *(char**)b);
}


void *threadRoutine(void *argumentPointer);

FILE* gradingC(FILE *answerFile, char*problemC, char*path_a);
static int check_thread_status(char *pnWorkStatus, int nWaitTime);
int list_dir(const char* path,char**saveName);
void ans_numSort(struct ans_num* num, int);
void subdirOutput(char *wd, char* studentId, char*wd_a, char*answerQ);
void programCompile(char*answerQ,char*path );
//....
char* studentFile[MAX_SIZE];
void EraseSpace(char *ap_string);

int innerFileCount;
int e_flag = 0;//e�ɼǽ� 1�ιٲ�
int p_flag = 0;//p�ɼǽ� 1�ιٲ�
int t_flag = 0;//t�ɼǽ� 1�ιٲ�
int h_flag = 0;//h�ɼǽ� 1�ιٲ�
int c_flag = 0;//c�ɼǽ� 1�ιٲ�
void optionE(char*);
void optionP();
void optionH();
void optionC(int count);
char* cName[MAX_SIZE];//c�ɼǽ� �߰��� �޴� �Ķ����
char eName[MAX_SIZE];//e�ɼǽ� �߰��� �޴� �Ķ����
char* tName[MAX_SIZE]={"12"};//t�ɼǽ� �߰��� �޴� �Ķ��?
int optCount1 = 0;//c�ɼ� �Ķ������ ����
int optCount2 = 1;//t�ɼ� �Ķ������ ����

void makeScore_table(char*, struct ans_num*, int);
void makeFinal_Score_table(char*stdId, struct ans_num* num, char* folder, int stdCount, int ansCount);


//______-----_____--------------------
//total exception!!!
//.exe .stdout .csv not count!!!!!!
//-------------------------------------
int main(int argc, char **argv)
{
	//-----
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);
	//---time

	//<---
	char* studentId[MAX_SIZE];
	char* answerQ[MAX_SIZE];

	//���� ������ ���� ����ó��
	if(argc==1){
		fprintf(stderr, "Usage : ssu_score <STDENTDIR> <TRUEDIR> [OPTION]\n");
		exit(1);
	}

	if (!strcmp(argv[1], "-c")) {
		int para = 2;
		while (argv[para] != NULL) {
			if (para == 7) {
				break;
			}
			strcpy(cName[para-2], argv[para]);
			para++;
			//after c -> save in array
		}
		c_flag=1;
		optionC(para-2);
	}

	if (!strcmp(argv[1], "-h")) {
		h_flag=1;
		optionH();
		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		exit(0);
	}


	//�ɼ� Ȯ��
	int c=0;

	char std_folder[MAX_SIZE];
	char ans_folder[MAX_SIZE];
	
	strcpy(std_folder,argv[1]);
	strcpy(ans_folder,argv[2]);

	int g;
int opt_t_oth;
	while ((c = getopt(argc, argv, "e:pt::hc:")) != -1) {
		printf("index: %d\n",optind);
		switch (c) {
			case 'e'://e�ɼ� �߻���
				e_flag = 1;
				//e�ɼ� �Լ��� gogo
				strcpy(eName, optarg); //e�ɼ� ���� �Ķ���ʹ� ����
				break;
			case 'p'://p�ɼ� �߻���
				p_flag = 1;
				break;
			case 't'://t�ɼ� �߻���
				t_flag = 1;
				
					if (optCount2 == 5) {
						break;
					}
					opt_t_oth=optind;



				//t�ɼ��Լ��� gogo
				//strcpy(tName,optarg);
				//���� �Ķ���ʹ� optarg
				break;
			case 'h'://h�ɼ� �߻���
				h_flag = 1;
				printf("h option!\n");

					printf("----%d\n",optind);
				break;
			case 'c'://c�ɼ� �߻���
				c_flag = 1;
				for (; optind < argc; optind++) {
					if (optCount1 == 5) {
						optionC(5);
					}
					strcpy(cName[optCount1] , argv[optind]);
					optCount1++;
				}
				optionC(optCount1);

				break;
			case '?'://���� �ش����� ���� �ɼ� �߻���
				printf("Unknown option %c\n", optopt);
				exit(1);
		}
		
		
	}

	//----------------------------------------------------------------------------------
	

	printf("%d\n",t_flag);

	//student
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
//
	//--------------------------------------------------------------------



	// ANS_DIR folder;
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

	if ((listcount = scandir(ans_folder, &namelist, NULL, alphasort)) == -1)
	{
		fprintf(stderr, "%s Directory Scan Err\n", ans_folder);
		exit(1);
	}



	for (i = 2; i < listcount; i++)
	{
		if (strstr(namelist[i]->d_name, ".csv") != NULL) {
			listcount--;
			continue;
		}
		//pathName = namelist[i]->d_name;
		answerQ[i - 2] = namelist[i]->d_name;
	}

	//answer to struct ans_num
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

	}
	//AND_DIR folder name sort
	ans_numSort(num, listcount - 2);

	for (i = 0; i < listcount - 2; i++) {
		strcpy(answerQ[i], num[i].name);
	}

	//���� �̸��� Ȯ���ڸ� �߰�
	for (i = 0; i < listcount - 2; i++) {
		if (num[i].name_s != 0) {
			sprintf(num[i].file_name, "%s.txt", num[i].name);
		}
		else {
			sprintf(num[i].file_name, "%s.c", num[i].name);
		}
	}


	//-------------- ans folder sort!

	//ansNum�� ������ ���� -> csv���� ����
	makeScore_table(ans_folder, num, listcount);

	//-----------------------------------------

	getcwd(path, 200);
	//���� ��ġ Ȯ��


	char* proNum_txt[MAX_SIZE];
	int problemT=0;
	char nPath[PATH_MAX];
	strcpy(nPath,path);
	for(i=2;i<listcount;i++){
		char* problemNum=namelist[i]->d_name;

		sprintf(nPath,"%s/%s/%s",path,ans_folder,namelist[i]->d_name);
		if(strchr(answerQ[i-2],'-')==NULL){

			//c file
			proNum[problemC]=namelist[i]->d_name;
			problemC++;
			programCompile(nPath,problemNum);////

		}
		else{
			//txt file
			proNum_txt[problemT]=namelist[i]->d_name;
			problemT++;
			//textfile compare function

		}
}


//---------------------------------------------------------------------

struct stat file_info;
getcwd(path,200);
struct dirent **nlist;
char*saveName[PATH_MAX];
int k;
char path1[MAX_SIZE];
int new_listcount;
for(j=0;j<num_s;j++){
	sprintf(path1,"%s/%s/%s",path,std_folder,studentId[j]);
	int ffd;

	new_listcount = list_dir(path1,saveName);

	///////////////////////////////////////////////////////////////////
	struct ans_num*num_sf;
	num_sf = (struct ans_num*)malloc(sizeof(struct ans_num)* new_listcount);

	char* text_f2;

	for (i = 0; i < new_listcount; i++) {
		strcpy(num_sf[i].name , saveName[i]);
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
			//num_sf[i].name_s = atoi(text_f2);
		}
		num_sf[i].name_i = atoi(saveName[i]);
	}

	ans_numSort(num_sf, new_listcount);

	for (i = 0; i < new_listcount; i++) {
		saveName[i] = num_sf[i].name;
	}
	///////////////////////////////////////////////
	char newpath[PATH_MAX];
	int newfd;
	for(i=0;i<new_listcount;i++){

		sprintf(newpath,"%s/%s",path1,saveName[i]);

		if(strchr(saveName[i],'-')==NULL){
			if(strstr(saveName[i],".c")!=NULL){
				char pronum[MAX_SIZE];
				strcpy(pronum,strtok(saveName[i],"."));
				//c file

				programCompile(path1,pronum);

				//saveName = 11 ,12,13,14

			}
		}	
		else{

			//txt file
			//textfile compare function
//--------------------------------------------------------//


		}

	}


	//-----------------------------------------------------------------------
}


//-------------------directory search end!!!!

//answer!
// proNum[] c file name
//problemC; -> c file count

//student
//num_s -> student number count
//studentId[] studentId name



printf("problem..?");
FILE *answerFile;
FILE *noSpaceAnsFile;
FILE *stdFile;
FILE *noSpaceStdFile;
char* path_ans;
strcpy(path_ans, path);

char no_path_a[MAX_SIZE];
sprintf(no_path_a, "no_spa_%s", path_ans);


for (i = 0; i < problemC; i++) {
	//answer path name -> answerFile open
	//1 answer file -> all student file grade!! for(  for() );
	noSpaceAnsFile = gradingC(answerFile, proNum[i], path_ans);
	noSpaceAnsFile = fopen(no_path_a, "r");

	for (j = 0; j < num_s; j++) {
		char* path_std; // student path name -> studentFile open
		char* stdId = studentId[i];
		char no_path_s[MAX_SIZE];
		sprintf(no_path_s, "no_spa_%s", path_std);

		noSpaceStdFile = gradingC(stdFile, proNum[i], path_std);
		noSpaceAnsFile = fopen(no_path_s, "r");

		//Large, small spell don't care -> compare

		if (noSpaceAnsFile == NULL || noSpaceStdFile == NULL) {
			printf("file open error\n");
			exit(1);
		}
		int state1, state2;

		// �ΰ��� ���Ͽ� ����� �����͸� ����
		while (1) {
			//�ΰ��� ���� ��� ���� �������� ���� ���
			if (feof(noSpaceAnsFile) == 0 && feof(noSpaceStdFile) == 0) {

				char strTemp[MAX_SIZE];
				char *pAnswerLine = NULL;
				char *pStdLine = NULL;

				pAnswerLine = fgets(strTemp, sizeof(strTemp), noSpaceAnsFile);
				pStdLine = fgets(strTemp, sizeof(strTemp), noSpaceStdFile);

				if (strcasecmp(pAnswerLine, pStdLine)) {
					printf("not same.\n");
					break;
				}
			}

			//�ϳ��� ���ϸ� ���� ������ ���
			else if (feof(noSpaceAnsFile) != 0 && feof(noSpaceStdFile) == 0) {
				printf("not same.\n");
				break;
			}

			//�ϳ��� ���ϸ� ���� ������ ���
			else if (feof(noSpaceAnsFile) == 0 && feof(noSpaceStdFile) != 0) {
				printf("not same.\n");
				break;
			}

			//�ΰ��� ���� ��� ���� ������ ���.
			//(ù ��° ���ǹ����� �� ������ ���ڴ� �˻��߱� ������
			//�� ������ ���ÿ� feof�� ���� Ż���ϸ� ������ ������ ��)
			else {
				printf("same.\n");
				break;
			}
		}

		//	 fclose�Լ��� ����� ������ �߻��ϸ�
		//	   0�� �ƴ� �ٸ����� �����ϹǷ� ������ ����� �ǴܵǸ�
		//	   �ȳ��� ���α׷��� ����
		state1 = fclose(noSpaceAnsFile);
		state2 = fclose(noSpaceStdFile);
		if (state1 != 0 || state2 != 0) {
			printf("stream close error\n");
			exit(1);
		}
		remove(path_std);
	}
	remove(path_ans);


}
/*
	//-----------------------------------------------result grading!!!

	//warning error count!!
	//for loop of student folder
	//scan -> stdout
	//compare "warning" / "error"
	// discount!!

	//------------------------------------------------------------------------------

	//�ɼ��Լ��� ����
	if (e_flag) {
		optionE(eName);
	}
	if (p_flag) {
		optionP();
	}
	if (c_flag) {
		optionC(optCount1);//...??
	}
	closedir(dir);
	closedir(dir2);

	if (h_flag) {
		optionH();
	}
*/
	printf("THE END\n");
	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}

//-----------------main end----------------------------------//

void makeScore_table(char* folder, struct ans_num* num, int listcount) {
	FILE *pFile;
	char pFilePath[MAX_SIZE];
	sprintf(pFilePath, "./%s/score_table.csv", folder);
	printf("%s\n", pFilePath);
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
			strcpy(b,strtok(NULL,",a"));
			num[i].score=atof(b);
			i++;
			if(strtok(NULL,",")=="\n")
				continue;
		}




	}
	fclose(pFile);
}

void makeFinal_Score_table(char* stdId, struct ans_num* num, char* folder, int stdCount, int ansCount) {
	int row, col;
	FILE *pFile;
	char pFilePath[MAX_SIZE];
	sprintf(pFilePath, "./%s/score.csv", folder);
	printf("final score : %s\n", pFilePath);
	int i,j;
	//if ((pFile = fopen(pFilePath, "r+")) == NULL) {
	pFile = fopen(pFilePath, "w+");
	fprintf(pFile, " ,");
	for (i = 0; i < ansCount; i++) {
		fprintf(pFile, "%s,", num[i].file_name);
	}
	fprintf(pFile, "sum\n");
	//1st col
	for (i = 0; i < stdCount; i++) {
		fprintf(pFile, "%d,",stdId[i]);
		for (j = 0; j < ansCount; j++) {
			fprintf(pFile, "%.2lf,", finalScore[i][j]);
		}
		fprintf(pFile, "%.2lf\n", sum[i]);
	}
	//otherwise...
	fclose(pFile);
}

void EraseSpace(char *ap_string)
{
	// p_dest �����͵� ap_string �����Ϳ� ������ �޸𸮸� ����Ų��.
	char *p_dest = ap_string;

	// ���ڿ��� ���� ���������� �ݺ��Ѵ�.
	while (*ap_string != 0) {
		// ap_string�� ����Ű�� ���� ���� ���ڰ� �ƴ� ��츸
		// p_dest�� ����Ű�� �޸𸮿� ���� �����Ѵ�.
		if (*ap_string != ' ') {
			if (p_dest != ap_string) *p_dest = *ap_string;
			// �Ϲ� ���ڸ� �����ϸ� ���� ������ ��ġ�� �̵��Ѵ�.
			p_dest++;
		}
		// ���� ���� ��ġ�� �̵��Ѵ�.
		ap_string++;
	}
	// ���ڿ��� ���� NULL ���ڸ� �����Ѵ�.
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

FILE* gradingC(FILE *answerFile, char*problemC, char*path_a) {
	FILE  *noSpaceAnsFile = NULL;

	//���� ��ȣ �ٿ��� ��Ʈ ����
	char no_path_a[MAX_SIZE];
	sprintf(no_path_a, "no_spa_%s", path_a);
	noSpaceAnsFile = fopen(no_path_a, "w");
	answerFile = fopen(path_a, "r"); //���� ��ġ
	if (answerFile != NULL)
	{
		char strTemp[MAX_SIZE];
		char *pStr = NULL;

		while (!feof(answerFile))
		{
			pStr = fgets(strTemp, sizeof(strTemp), answerFile);
			EraseSpace(pStr);
			//pStr�� ���ο� �ؽ�Ʈ�� ����!
			fprintf(noSpaceAnsFile, "%s", pStr);
		}
		fclose(answerFile);
		fclose(noSpaceAnsFile);
		return noSpaceAnsFile;
	}
	else
	{
		//���� ó��
		return NULL;
	}

}

void programCompile(char* path,char* problemNum){
	char* gccname="gcc -o";
	char name[PATH_MAX];
	int i;
	pthread_t threadID;

	// threadID�� TID�� �޾ƿ���, threadRoutine��� �Լ� �����ͷ� �����带 �����Ѵ�.
	char checkname[PATH_MAX];
	sprintf(checkname,"%s/%s.exe",path,problemNum);
	
	sprintf(name,"%s %s %s/%s.c",gccname,checkname,path,problemNum);
	//gcc -o .exe .c
	
	int check;
	int fd2,bk,bk2;
	bk=open("dummy",O_WRONLY|O_CREAT);
	bk2=open("dummy2",O_WRONLY|O_CREAT);
	close(bk);
	close(bk2);
	dup2(1,bk);
	dup2(2,bk);
	char res[PATH_MAX];

	sprintf(res,"%s/%s.stdout",path,problemNum);
	if((fd2=open(res,O_WRONLY|O_CREAT|O_TRUNC))==-1){
		fprintf(stderr, "open error for %s\n",res);
		exit(1);
	}
		fchmod(fd2,0777);
	dup2(fd2,1);
	dup2(fd2,2);


	system(name);
	//gcc -o ~~~

	char argument[MAX_SIZE];
	sprintf(argument, "%s", checkname);
	int ch=0;
	if((check=access(checkname,F_OK))<0){
		if(t_flag){
			int h;
	dup2(bk,1);
			dup2(bk,2);
			
			close(fd2);
			remove(res);

	if((fd2=open(res,O_WRONLY|O_CREAT|O_TRUNC))==-1){
		fprintf(stderr, "open error for %s\n",res);
		exit(1);
	}
		fchmod(fd2,0777);
	dup2(fd2,1);
	dup2(fd2,2);
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
			dup2(bk,2);
			remove("dummy");
			remove("dummy2");
			close(fd2);
			remove(res);

		}
	}

	if(ch==0){
		pthread_create(&threadID, NULL, threadRoutine, (void*)argument);
		pthread_detach(threadID);


		if (!check_thread_status(argument, TIME_LIMIT)) {
			pthread_cancel(threadID);
		}
		dup2(bk,1);
		dup2(bk,2);
		remove("dummy");
		remove("dummy2");
		close(fd2);
	}
	// ~~~.stdout making...
}

void *threadRoutine(void *argumentPointer)
{
	char *argument = (char *)argumentPointer;

	system(argument);

	strcpy(argument, "end");

	// �θ� ������ �κп��� ���ϰ��� �ޱ⶧���� �׻� ������ ���ش�.
	return NULL;
}

static int check_thread_status(char *pnWorkStatus, int nWaitTime){
	int i;
	// �־��� nWaitTime ��ŭ�� ���
	for (i = 0; i < nWaitTime; i++){

		// �����尡 �Ϸ�� ���������� *pnWorkStatus�� 1�� �ȴ�.
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

	//e�ɼ� ������ ���� ���ڸ��� ������ �ִ��� Ȯ��. ������ ���� ����
	res = access(err, 0);
	if (res == -1) {
		mkdir(err, 0777);
	}

	//eName/�й�/������ȣ_error.txt�� �����޽��� ���


}

void optionP() {

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
	// ���� ���丮�� ����.
	if ((dirp = opendir(".")) == NULL) {
		printf("error: opendir..\n");
		exit(1);
	}
	// ���� ���丮�� ��� �׸��� �д´�.
	while (dentry = readdir(dirp)) {
		// ���丮�� �׸��� ���̳���ȣ�� 0 �ƴ� ���� ã�´�.
		// ���̳���ȣ�� 0 �̸� �� ���� ������ �� ���̴�.
		if (dentry->d_ino != 0) {

			// �о�ȥ �׸��� "."(������丮)�� ".."(�θ���丮)�� �ǳʶڴ�.
			// �̴� ���� ���丮�� Ž���ϴ� ���̱� ������ �����ϴ� ���̴�.
			if ((!strcmp(dentry->d_name, ".")) || (!strcmp(dentry->d_name, "..")))
				continue;

			// ���� �׸��� ���������� �����´�.
			stat(dentry->d_name, &fstat);

			// ���� �׸��� ���°� ���丮�� ��� �ش� ���丮�� �̵�
			if (S_ISDIR(fstat.st_mode)) {
				chdir(dentry->d_name);
			}
			else if(S_ISREG(fstat.st_mode)){
				if (!strcmp(dentry->d_name, "score.csv")) {
					strcpy(path, dentry->d_name);
					check = 1;
					break;
					// ���� �׸��� ���°� �Ϲ������� ���
					// score.csv ã��. ã���� while�� Ż��
				}

			}
		}
	}
	// ���� ���� ���丮�� ����� �������Ƿ� �ݾ��ش�.
	closedir(dirp);

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



	/*
	   int j;
	   for (j = 0; j < innerFileCount; j++) {
	   printf("%s\n", studentFile[j]);
	   }
	   */



	closedir(dirp);

}
void ans_numSort(struct ans_num* num, int cnt) {
	//sort
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
