#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define DEBUG

#define true 1
#define false 0
typedef int bool;

 20 // 전역변수
 21 // 동시에 여러 개의 파일에 대해 백업이 수행되어야 하기 때문에 메시지 출력 시 반드시 mutex를 사용하여 동기화
 22 pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
 23 int startfile_fin = 0;
 24 int dir_cnt = 0;
 25 bool isnewfile = false;
 26 char backup_dir[255];           // 백업할 경로
 27 char backup_log_file[255];      // 백업 로그 파일 이름
 28 FILE *fp_log_file;              // 로그 파일 FILE 포인터 변수
 29 
 29 
 30 // add 옵션 구조체 - 옵션 정보 보관용
 31 typedef struct ADD_OPTION { 
 32         char opt_print[10]; // ex) m,n,t,d
 33 
 34         int m_flag; // m옵션이면 1, 아니면 0
 35 
 36         int n_flag; // n옵션이면 1, 아니면 0
 37         int n_number; // n_number 저장
 38 
 39         int t_flag; // t옵션이면 1, 아니면 0
 40         int t_time; // t_time 저장
 41         
 42         int d_flag; // d옵션이면 1, 아니면 0
 43         char d_directory[1024]; // d_directory 저장
 44 } add_option;
 45 
 46 // 백업 리스트 - 링크드 리스트로 구현
 47 typedef struct BACKUP_LIST {
 48         struct BACKUP_LIST *next;
 49         pthread_t tid;
 50         char f_filename[242];   // 파일의 절대경로, 크기는 255-13 (_YYMMDDHHMMSS부분 제외)
 51         char p_filename[242];   // 파일 이름, 파일 이름은 [파일이름_시간]으로 작성하며, 이때 파일이름 부분에 해당, 따라서 크기는 최대 255-13
 52         int period;             // 파일의 백업 주기
 53         add_option opt_data;    // 옵션 정보 보관
 54 } backup_list;
 55 
 56 // 백업 리스트의 헤더와 테일 - 링크드 리스트로 구현
 57 backup_list *HEAD = NULL, *TAIL = NULL;
 58 

// 함수 원형
void thread_function(void *arg);
void backup_function(char *filename);
void print_log(char *name, int size, time_t mt, int index);

// ssu_backup 실행시 초기화하는 함수
void ssu_backup_start(int argc, char *argv[]);

// 프롬포트 함수
void prompt(char *b_dir);

// 명령어 함수
void add(char *token[10], int argc);
void removes(char *token[10], int argc);
void compare(char *token[10], int argc);
void recover(char *token[10], int argc);
void list();

// 상대경로를 절대경로로 변환하여 주는 함수
void rpath_to_fpath(char *r_path, char *f_path) {
	if (r_path[0] != '/') {
		getcwd(f_path, 1024);
		strcat(f_path, "/");
		strcat(f_path, r_path);
	}
	else {
		strcpy(f_path, r_path);
	}
}

// 백업 리스트에서 파일을 검색하는 함수, 파일이 있으면 0을 리턴하고, 파일이 없으면 -1을 리턴한다.
int search_list(char *filename) {
	backup_list *cur;
	for (cur = HEAD; cur != NULL; cur = cur->next) {
		if (strcmp(cur->filename, filename) == 0)
			return 0;
	}
	return -1;
}

// 백업 리스트에서 특정 파일을 삭제하는 함수
void delete_list(char *filename) {
	backup_list *cur;
	backup_list *delete_this;
	// HEAD 검사 후 HEAD가 지워지면 갱신
	cur = HEAD;
	if (strcmp(HEAD->filename, filename) == 0) {
		delete_this = cur;
		HEAD = HEAD->next;
		free(delete_this);
		return;
	}

	// HEAD는 지워지지 않음
	for (cur = HEAD; (cur != NULL && cur->next != NULL); cur = cur->next) {
		if (strcmp(cur->next->filename, filename) == 0) {
			delete_this = cur->next;
			cur->next = cur->next->next;
			// TAIL 검사 후 TAIL이 지워지는 거면 갱신
			if (cur->next->next == NULL) {
				TAIL = cur;
			}
			free(delete_this);
			return;
		}
	}
#ifdef DEBUG
	printf("%s가 백업 리스트에 없음!\n", filename);
#endif
	return;
}

// 백업 리스트를 초기화하는 함수
void default_list() {
	backup_list *cur;
	while (HEAD != NULL) {
		cur = HEAD;
		HEAD = HEAD->next;
		free(cur);
	}	
	HEAD = NULL, TAIL = NULL;
}

// 백업 리스트에 새로 추가하는 함수, 쓰레드를 하나 생성하는 함수
backup_list *make_backup_list(char filename[242], int period, char option[10]) {
	backup_list *temp = (backup_list *)malloc(sizeof(backup_list));
	strcpy(temp->filename, filename);
	temp->period = period;
	strcpy(temp->option, option);
	temp->next = NULL;

	if (pthread_create(&(temp->tid), NULL, (void *)(&thread_function), (void *)filename) != 0) {
		fprintf(stderr, "pthread_create error\n");
	}
	return temp;
}

// 새로 생성된 쓰레드가 수행할 함수, 새로 생성된 쓰레드의 시작지점
void thread_function(void *arg) {
	char *data = (char *)arg;
	backup_function(data);
}

// 백업을 수행하는 함수 - add할수 있게 수정
void backup_function(char *filename) {
	struct stat src_sc;
	time_t old = 0, new = 0;
	int i = 0;
	int check = 0;
	while (1) {
		stat(filename, &src_sc);
		new = src_sc.st_mtime;

		if (new == old) {
			continue;
		}

		if (old == 0) {
			if (!isnewfile)
				;
			else
				print_log(filename, src_sc.st_size, new, 1);
		}
		else if (old != new) {
			print_log(filename, src_sc.st_size, new, 2);
		}

		old = new;
		if (!isnewfile)
			startfile_fin++;
	}
}

// 로그 파일에 출력하는 함수
void print_log(char *name, int size, time_t mt, int index) {
	time_t timer = time(NULL);
	struct tm *t = localtime(&timer);
	char temp[14];
	sprintf(temp, "_%02d%02d%02d%02d%02d%02d", t->tm_year % 100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	struct tm *tt = localtime(&mt);

	pthread_mutex_lock(&mutex);
	switch(index) {
		case 1: // added
			printf("%s added\n", name);
			break;
		case 2: // generated
			printf("%s%s generated\n", name, temp);
			break;
		case 3: // deleted
			printf("%s deleted\n", name);
			break;
	}
}

// main함수
int main(int argc, char *argv[])
{
	ssu_backup_start(argc, argv);
	prompt(argv[1]);

	exit(0);
}

// ssu_backup 실행시 인자를 입력받음
void ssu_backup_start(int argc, char *argv[]) {
	
	// 상대경로와 절대경로 모두 입력 가능
	
	// 인자가 없으면 current working 디렉토리 밑에 백업 디렉토리 생성, 로그파일 생성
	if (argc == 1) {
		system("mkdir backup");
		strcpy(backup_dir, "backup");
	}

	// 인자가 2개 이상이면 usage 출력 후 프로그램 종료
	if (argc > 2) {
		fprintf(stderr, "usage : ./ssu_backup <DIR>\n");
		exit(0);
	}
	
	// 인자로 입력받은 디렉토리를 찾을 수 없으면 usage 출력 후 프로그램 종료
	// 인자로 입력받은 디렉토리가 디렉토리 파일이 아니라면 usage 출력 후 프로그램 종료
	// 인자로 입력받은 디렉토리의 접근권한이 없는 경우 usage 출력 후 프로그램 종료
	struct stat file_info;
	if ((lstat(argv[1], &file_info)) < 0) {
		fprintf(stderr, "stat error - don't find %s (directory)\n", argv[1]);
		exit(1);
	}

	if (argc == 2) {
		if (access(argv[1], F_OK) != 0 || (!S_ISDIR(file_info.st_mode)) || access(argv[1], R_OK) != 0 || access(argv[1], W_OK) != 0 ) {
			fprintf(stderr, "usage : ./ssu_backup <DIR>\n");
			exit(0);
		}

		// 모든 조건에 부합할 경우, 인자로 입력받은 디렉토리 안에 백업 디렉토리 생성
		char temp[255] = {"mkdir "};
		strcat(temp, argv[1]);
		strcat(temp, "/backup");
		system(temp);
		strcpy(backup_dir, temp);
	}
}

// 프롬포트 함수
void prompt(char *b_dir) {
	// 위 과정을 모두 정상적으로 통과하였다면 다음과 같은 프롬포트 출력 -> while 무한루프문 사용
	
	while(1) {
		// 프롬포트 모양 : 공백없이 학번, '>' 문자 출력 ex) 20150413>
		printf("20150413>");

		// 프롬포트에서 실행 가능 명령어 : add, remove, compare, recover, list, ls, vi, vim, exit
		// 이외 명령어 입력 시 에러 처리 후 프롬포트로 제어가 넘어감 (명령어 실행 중지하고 입력받는 상황으로 돌아감)
		char input[512];
		char input_o[512];
		char *token[10]; // 수정이 필요해 보임 (char token[1000][1000]?) - 근데 strtok은 *만 받으니 적절히 고쳐야 함
		// *변수만 두고 [][]변수 두는 것도 고려하기
		fgets(input, 512, stdin);

		// 엔터만 입력시 프롬포트 재출력
		if (strcmp(input, "\n") == 0) {
			continue;
		}

		strcpy(input_o, input);

		// 공백을 기준으로 토큰분리해서, 맨 첫번째 글자가 다음 명령어일 경우 상황에 맞게 수행한다.
		token[0] = strtok(input, " ");
		
		int argc = 1;
		while(argc < 10) {
			token[argc] = strtok(NULL, " ");
			if (token[argc] == NULL) {
				for (int i = 0; ; i++) {
					// 입력의 맨 마지막으로 들어오는 '\n'을 없애자
					if (token[argc-1][i] == '\n')
						token[argc-1][i] = '\0';

					if (token[argc-1][i] == '\0')
						break;
				}
				break;
			}
			argc++;
		}

		// 만약 10개를 넘는 token이 들어올 경우 에러 처리
		// token은 최대 10개를 넘을 수 없음. 그 이유는 다음과 같음
		// 가장 긴 명령어를 생각해 보면, add <filename> [period] -m -n number -t time -d directory -> token은 10개
		if (argc == 10) {
			if (strtok(NULL, " ") != NULL) {
				fprintf(stderr, "too many input value. You may use OPTION wrong. Please check it.\n");
				continue;
			}
		}

		// add 명령 - add <filename> [PERIOD] [OPTION]
		// <filename>이 없을 시 에러
		// [PERIOD] : 5,6,7,8,9,10의 정수형만 허용, 실수형 입력 시 에러 처리 후 프롬포트로 제어 넘김
		// [OPTION] : -m, -n NUMBER, -t TIME, -d DIRECTORY
		
		if (strstr(token[0], "add") != 0) {
#ifdef DEBUG
			printf("add 함수 호출!\n");
#endif
			add(token, argc);
			continue;
		}

		// remove 명령 - remove <FILENAME> [OPTION]
		// <filename>이 없을 시 에러 단 a옵션은 예외
		// [OPTION] : -a
		
		if (strstr(token[0], "remove") != 0) {
#ifdef DEBUG
			printf("remove 함수 호출!\n");
#endif
			removes(token, argc);
			continue;
		}

		// compare 명령 - compare <FILENAME1> <FILENAME2>

		if (strstr(token[0], "compare") != 0) {
#ifdef DEBUG
			printf("compare 함수 호출!\n");
#endif
			compare(token, argc);
			continue;
		}

		// recover 명령 - recover <FILENAME> [OPTION]
		// [OPTION] : -n <NEWFILE>
		
		if (strstr(token[0], "recover") != 0) {
#ifdef DEBUG
			printf("recover 함수 호출!\n");
#endif
			recover(token, argc);
			continue;
		}

		// list 명령 - list
		
		if (strstr(token[0], "list") != 0) {
#ifdef DEBUG
			printf("list 함수 호출!\n");
#endif
			// 만약 list 뒤에 다른 인자가 있다면, usage 출력 후 프롬포트로 제어를 넘김
			// 구현할 것
			if (argc > 1) {
				fprintf(stderr, "usage : \n");
				continue;
			}
			list();
			continue;
		}
		
		// ls 명령 - ls (2015-05-13 완료)	
		if (strstr(token[0], "ls") != 0) {
			system(input_o);
			continue;
		}

		// vi, vim 명령 - vi, vim (2015-05-13 완료)
		if (strstr(token[0], "vi") != 0 || strstr(token[0], "vim") != 0) {
			system(input_o);
			continue;
		}

		// exit 명령
		// exit 명령이 입력될 때까지 위에서 지정한 실행 가능 명령어를 입력받아 실행
		// exit 명령이 입력되면 현재 실행중인 모든 백업을 종료하고 프로그램 종료 -> 모든 백업 종료하는 걸 구현해야 함
		// 그냥 exit(0);으로 될까? -> 된다!
		if (strstr(token[0], "exit") != 0) {
			break;
		}

		// 그 이외 명령이 들어올 경우, 에러 처리 후 프롬포트로 제어를 넘김
		fprintf(stderr, "command error!\n");
		fprintf(stderr, "usable command: add, remove, compare, recover, list, ls, vi(m), exit\n");
	}

	exit(0);
}

// add 명령 - add <filename> [PERIOD] [OPTION]
// <filename> 이 없을 시 에러, [PERIOD] 입력 없을 시 에러
// [PERIOD] : 5, 6, 7, 8, 9, 10의 정수형만 허용, 실수형 입력 시 에러 처리 후 프롬포트로 제어 넘김
// [OPTION] : -m, -n NUMBER, -t TIME, -d DIRECTORY
void add(char *token[10], int argc)
{
	// <filename>이 있는지 확인 - 1. 인자의 개수가 1개이면 에러
	if (argc < 3) {
		fprintf(stderr, "usage: add <filename> <PERIOD> [OPTION]\n");
		return;
	}
	
	// <filename> : token[1]
	// <filename>이 존재하는지 확인 - 존재하지 않으면 에러 처리
	if (access(token[1], F_OK) != 0) {
		fprintf(stderr, "%s doesn't exist!\n", token[1]);
		return;
	}
		
	// <filename>이 일반 파일인지 확인 - 아니라면 에러 처리
	struct stat statbuf;
	if (lstat(token[1], &statbuf) < 0) {
		fprintf(stderr, "lstat error for %s\n", token[1]);
		return;	
	}

	// <filename>이 이미 백업 리스트에 존재하는지 확인 - 존재한다면 에러 처리
	// 단 이때 filename은 절대경로
	char filename[1024];
	// 만약 <filename> 이 상대경로일 경우 절대경로를 filename에 저장
	
	rpath_to_fpath(token[1], filename);
/*
	if (token[1][0] != '/') {
		getcwd(filename, 1024);
		strcat(filename, "/");
		strcat(filename, token[1]);
	}
	else {
		// 절대경로이면 그냥 filename에 저장
		strcpy(filename, token[1]);
	}
*/
	// 백업 리스트 순회,  백업 리스트에 존재하면 에러처리
	if (search_list(filename) == 0) {
		fprintf(stderr, "%s is already exist in backup_list!\n", token[1]);
		return;
	}

	// [PERIOD] : token[2]함
	// [PERIOD]가 실수형인지 확인, [PERIOD]가 5,6,7,8,9,10인지 확인 -> 후자 확인으로 같이 할 수 있음
	if ((strcmp(token[2], "5") != 0) && (strcmp(token[2], "6") != 0) && (strcmp(token[2], "7") != 0) && (strcmp(token[2], "8") != 0) && (strcmp(token[2], "9") != 0) && (strcmp(token[2], "10") != 0)) {
		fprintf(stderr, "[PERIOD] is 5 to 10 in integer value.\n");
		return;	
	}
	
	int period = atoi(token[2]);

	// [OPTION] : token[3] ~ token[argc-1] 사이
	// 옵션 구분지어주기 (flag의 경우 있으면 1 없으면 0)
	int m_flag = 0;

	int n_flag = 0;
	int n_number = 0;	// n_number의 경우 1 <= NUMBER <= 100

	int t_flag = 0;
	int t_time = 0; 	// t_time의 경우 60 <= TIME <= 120
	
	int d_flag = 0;
	char d_directory[1024];
	for (int i = 3; i < argc; i++) {
		
		if (strcmp(token[i], "-m") == 0) {
#ifdef DEBUG
			printf("add명령어에 m옵션 감지!\n");
#endif
			m_flag = 1;
#ifdef DEBUG
			printf("m_flag: %d\n", m_flag);
#endif
		}
		else if (strcmp(token[i], "-n") == 0) {
#ifdef DEBUG
			printf("add명령어에 n옵션 감지!\n");
#endif
			n_flag = 1;

			// n옵션의 경우 NUMBER 입력이 없으면 예외 처리 후 프롬포트로 제어를 넘김
			i++;
			if (i == argc || strstr(token[i], ".") != 0 || strstr(token[i], "-") != 0 || atoi(token[i]) == 0) {
				fprintf(stderr, "-n error!\nusage: -n NUMBER, buf NUMBER is not exist or not positive integer.\n");
				return;
			}

			// NUMBER가 1 <= NUMBER <= 100이 아니면 에러 처리
			n_number = atoi(token[i]);
			if (!(n_number >= 1 && n_number <= 100)) {
				fprintf(stderr, "-n error!\nusage: -n NUMBER, and NUMBER is 1<=NUMBER<=100 integer value. But you doesn't.\n");
				return;
			}
#ifdef DEBUG
			printf("n_flag: %d, n_number: %d\n", n_flag, n_number);
#endif
		}
		else if (strcmp(token[i], "-t") == 0) {
#ifdef DEBUG
			printf("add명령어에 t옵션 감지!\n");
#endif

			t_flag = 1;

			// t옵션의 경우 TIME의 입력이 없으면 예외 처리 후 프롬포트로 제어를 넘김
			i++;
			if (i == argc || strstr(token[i], ".") != 0 || strstr(token[i], "-") != 0 || atoi(token[i]) == 0) {
				fprintf(stderr, "-t error!\nusage: -t TIME, buf TIME is not exist or not positive integer.\n");
				return;
			}

			// TIME은 정수형이며 초를 나타냄. 범위는 60 <= TIME <= 120
			t_time = atoi(token[i]);
			if (!(t_time >= 60 && t_time <= 120)) {
				fprintf(stderr, "-t error!\nusage: -t TIME, and TIME is 60<=TIME<=120 interger value. But you doesn't.\n");
				return;
			}

#ifdef DEBUG
			printf("t_flag: %d\n, t_time: %d\n", t_flag, t_time);
#endif
		}
		else if (strcmp(token[i], "-d") == 0) {
#ifdef DEBUG
			printf("add명령어에 d옵션 감지!\n");
#endif

			d_flag = 1;

			// d옵션의 경우 한번에 인자로 줄 수 있는 디렉토리는 최대 1개
			// 디렉토리 주소를 저장해 놓자
			i++;
			if (i == argc) {
				fprintf(stderr, "-d error!\nusage: -d DIRECTORY, but DIRECTORY is not exist!\n");
				return;
			}

			// DIRECTORY가 디렉토리가 아닐 경우 에러 처리 후 프롬포트로 제어가 넘어감
			struct stat statbuf2;
			if (lstat(token[i], &statbuf2) < 0) {
				fprintf(stderr, "-d error! lstat error for %s\n", token[i]);
				return;
			}
			
			if (!S_ISDIR(statbuf2.st_mode)) {
				fprintf(stderr, "-d error! %s isn't DIRECTORY.\n", token[i]);
				return;
			}

			// DIRECTORY 추가
			strcpy(d_directory, token[i]);

#ifdef DEBUG
			printf("d_flag: %d, d_directory: %s\n", d_flag, d_directory);
#endif
		}
		else {
			// 잘못된 옵션을 사용하였으므로 에러 처리
			fprintf(stderr, "use wrong OPTION! usable option is -m, -n NUMBER, -t TIME, -d DIRECTORY\n");
			return;
		}

	}

	// add하는 함수 연결 - pthread 사용해서 링크드리스트에 add하고 백업 시작!
	//
	//
	// 구현하기!!!!!!
	//
	//

	return;
}

// remove 명령 : remove <FILENAME> [OPTION]
// 백업 리스트에 존재하는 파일(FILENAME)의 백업을 중단하기 위해 백업 리스트에서 삭제!
// 그러면 쓰레드가 백업 리스트에 자기가 없는거 보고 알아서 종료할 거임!
// <FILENAME>이 있거나 -a 옵션이 있거나 (a 옵션 입력 시 FILENAME은 입력하지 않으며 입력하면 에러)
// <FILENAME> : 백업을 하지 않을 (중단할) 파일의 경로,
// 백업을 중단할 파일이 백업 리스트에 존재하지 않을 시, 에러 처리 후 프롬포트로 제어가 넘어감
void removes(char *token[10], int argc)
{
	// 인자의 개수가 1개 이상이면 오류 (remove <FILENAME> 또는 remove -a이어야 함)
	if (argc != 2) {
		fprintf(stderr, "usage: remove <FILENAME> or remove -a\n");
		return;
	}

	// 인자가 -a옵션일 경우
	// 백업 리스트 초기화
	if (strcmp(token[1], "-a") == 0) {
#ifdef DEBUG
		printf("remove -a옵션!\n");
#endif
		default_list();
#ifdef DEBUG
		printf("remove -a옵션 완료!\n");
#endif
	}
	else {
#ifdef DEBUG
		printf("remove <FILENAME> 옵션, filename : %s\n", token[1]);
#endif
		// 인자가 FILENAME인 경우
		// 그 인자가 백업 리스트에 존재하지 않으면 에러 처리 (이러면, 인자가 FILENAME인지 아닌지도 확인 가능)
		// 단 그 인자는 절대경로이여야 함, 상대경로가 들어오면 절대경로로 변환해야 함
		char fpath[1024];
		rpath_to_fpath(token[1], fpath);
		if (search_list(fpath) != 0) {
			fprintf(stderr, "%s isn't exist backup_list!\n", token[1]);
			return;
		}

		// 백업 리스트에 존재하면 삭제
		delete_list(fpath);
#ifdef DEBUG
		printf("remove <FILENAME> 옵션 완료!\n");
#endif
	}
	return;
}

// compare 명령 : compare <FILENAME1> <FILENAME2>
// 이때 compare의 FILENAME은 절대경로만 입력됨 (Q&A 답변)
// FILENAME이 존재하지 않거나 일반 파일이 아니면 에러 처리 후 프롬포트로 제어가 넘어감
// 입력 인자가 2개가 아닐 경우 에러 처리 후 프롬포트로 제어가 넘어감
// 두 파일이 동일한 경우는, mtime과 파일 크기가 같다는 것을 의미 -> 표준출력으로 동일하다는 문구 출력
// 두 파일이 동일하지 않은 경우는 표준출력으로 각 파일의 mtime과 파일 크기를 출력
void compare(char *token[10], int argc)
{
#ifdef DEBUG_COMPLETE
	for (int i = 0; i < argc; i++) {
		printf("token[%d] : %s\n", i, token[i]);
	}
#endif

	// 입력 인자가 2개인지 확인하여 아니면 에러처리 (compare까지 실질적으로는 3개)
	if (argc != 3) {
		fprintf(stderr, "usage: compare <FILENAME1> <FILENAME2>\n");
		return;
	}
	
	// FILENAME1과 FILENAME2가 존재하는지 확인
	// token[1] : FILENAME1, token[2] : FILENAME2
	if (access(token[1], F_OK) != 0) {
		fprintf(stderr, "Doesn't exist %s\n", token[1]);
		return;
	}

	if (access(token[2], F_OK) != 0) {
		fprintf(stderr, "Doesn't exist %s\n", token[2]);
		return;
	}
	
	// stat 구조체 선언
	struct stat statbuf1, statbuf2;
	if (lstat(token[1], &statbuf1) < 0) {
		fprintf(stderr, "lstat error for %s\n", token[1]);
		return;
	}

	if (lstat(token[2], &statbuf2) < 0) {
		fprintf(stderr, "lstat error for %s\n", token[2]);
		return;
	}

	// 두 파일이 일반 파일인지 확인
	if (!S_ISREG(statbuf1.st_mode)) {
		fprintf(stderr, "%s isn't regular file\n", token[1]);
		return;
	}

	if (!S_ISREG(statbuf2.st_mode)) {
		fprintf(stderr, "%s isn't regular file\n", token[2]);
		return;
	}

	// 두 파일이 동일한지 여부 검사 - st_mtime, st_size 비교하여 같으면 표준 출력으로 같다고 함
	if ((statbuf1.st_mtime == statbuf2.st_mtime) && (statbuf1.st_size == statbuf2.st_size)) {
		printf("%s와 %s는 동일한 파일입니다!\n", token[1], token[2]);
	}
	else {
		// 다르면 각 파일의 mtime과 파일 크기를 출력
		printf("%s's mtime : %ld, file size : %ld bytes\n", token[1], statbuf1.st_mtime, statbuf1.st_size);
		printf("%s's mtime : %ld, file size : %ld bytes\n", token[2], statbuf2.st_mtime, statbuf2.st_size);
	}
	return;
}

// recover 명령 - recover <FILENAME> [OPTION]
// FILENAME의 백업 파일을 사용하여 현재의 파일을 백업된 파일로 변경
// 백업 파일이 존재하면, 백업파일의 백업시간(YYMMDDHHMMSS), 파일크기를 리스트 형태로 백업시간 기준 오름차순 출력
// 리스트는 사용자가 선택할 수 있도록 순번이 있으며, 파일을 선택하지 않는 경우(순번)도 생성
// [OPTION] : -n <NEWFILE> 
void recover(char *token[10], int argc)
{
	int n_flag = 0;
	char n_filename[242];
	// token[1]에는 <FILENAME>이 들어와야 함 -> 필수사항
	// token[2]에는 -n이, token[3]에는 <NEWFILE>이 들어와야 함-> 이건 선택사항
	if (argc > 4) {
		fprintf(stderr, "usage: recover <FILENAME> [OPTION]\nOPTION : -n <NEWFILE>\n");
		return;
	}

	// <FILENAME>이 존재하지 않으면 에러 처리
	if (argc < 2) {
		fprintf(stderr, "usage: recover <FILENAME> [OPTION]\nFILENAME don't exist!\n");
		return;
	}	

	// NEWFILE의 입력이 있는지 확인(따라서 argc가 3이면 에러)
	if (argc == 3) {
		fprintf(stderr, "usage: recover <FILENAME> [OPTION]\nOPTION : -n <NEWFILE>\n");
		return;
	}
	if (argc == 4) {
		// -n 옵션이 있는지 확인
		if (strcmp(token[2], "-n") != 0) {
			fprintf(stderr, "recover error\nwrong command, can use -n only\n");
			return;
		}

		// NEWFILE이 이미 존재할 경우 에러 처리		
		if (access(token[3], F_OK) == 0) {
			fprintf(stderr, "recover error\n,%s is already exist!\n", token[3]);
			return;
		}

		n_flag = 1;
		char fpath[242];
		rpath_to_fpath(token[3], fpath);
		strcpy(n_filename, fpath);

	}

	// <FILENAME>이 현재 백업 리스트에 존재한다면, 백업 수행 종료 후 복구 진행
	// 백업 수행 종료 = 백업 리스트에서 삭제로 간주하고 진행하였음 (답변 올라오는거 보고 진행하자)
	
	//
	// 구현해보기!!
	// 

	// 만약 변경할 파일에 대한 백업 파일이 존재하지 않으면 에러 처리
	
	//
	// 구현해보기!!
	//

	return;
}

// 현재 백업 실행중인 모든 백업 리스트 출력
// 한 줄에 한 개 파일의 절대경로, PERIOD, OPTION 출력
void list()
{
	backup_list *cur;
	for (cur = HEAD; cur != NULL; cur = cur->next) {
		printf("%s, %d, %s\n", cur->filename, cur->period, cur->option);
	}
	return;
}


237 // 로그 파일에 출력하는 함수
238 // 입력 매개변수 : 파일 이름 or 절대경로, 출력번호 (1 : added, 2 : generated, 3 : deleted, 4 : recovered)
239 void print_log(char *name, int index) {
240         time_t timer = time(NULL);
241         struct tm *t = localtime(&timer);
242         char temp1[20];
243         char temp2[20];
244         char temp_final[300];
245         sprintf(temp1, "[%02d%02d%02d %02d%02d%02d] ", t->tm_year % 100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm    _min, t->tm_sec);
246         sprintf(temp2, "_%02d%02d%02d%02d%02d%02d", t->tm_year % 100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_mi    n, t->tm_sec);
247 
248         pthread_mutex_lock(&mutex);
249         fp_log_file = fopen(backup_log_file, "a+");
250         switch(index) {
251                 case 1: // added
252 #ifdef DEBUG
253                         printf("%s%s added\n", temp1, name);
254 #endif
255                         sprintf(temp_final, "%s%s added\n", temp1, name);
256                         fwrite(temp_final, 1, sizeof(char)*strlen(temp_final), fp_log_file);
257                         break;
258                 case 2: // generated
259 #ifdef DEBUG
260                         printf("%s%s%s generated\n", temp1, name, temp2);
261 #endif
262                         sprintf(temp_final, "%s%s%s generated\n", temp1, name, temp2);
263                         fwrite(temp_final, 1, sizeof(char)*strlen(temp_final), fp_log_file);
264                         break;
265                 case 3: // deleted (remove)
266 #ifdef DEBUG
267                         printf("%s%s deleted\n", temp1, name);
268 #endif
269                         sprintf(temp_final, "%s%s deleted\n", temp1, name);
270                         fwrite(temp_final, 1, sizeof(char)*strlen(temp_final), fp_log_file);
271                         break;
272                 case 4: // recovered
273 #ifdef DEBUG
274                         printf("%s%s recovered\n", temp1, name);
275 #endif
276                         sprintf(temp_final, "%s%s recovered\n", temp1, name);
277                         fwrite(temp_final, 1, sizeof(char)*strlen(temp_final), fp_log_file);
278                         break;
279         }
280         fclose(fp_log_file);
281         pthread_mutex_unlock(&mutex);
282 }
