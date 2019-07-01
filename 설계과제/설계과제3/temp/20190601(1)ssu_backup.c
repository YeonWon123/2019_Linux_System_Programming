#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#define DEBUG

#define true 1
#define false 0
typedef int bool;

// 전역변수
// 동시에 여러 개의 파일에 대해 백업이 수행되어야 하기 때문에 메시지 출력 시 반드시 mutex를 사용하여 동기화
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int startfile_fin = 0;
int dir_cnt = 0;
bool isnewfile = false;
char backup_dir[255]; 		// 백업할 경로 - 절대경로로 저장해야 할듯
char backup_log_file[255]; 	// 백업 로그 파일 이름
FILE *fp_log_file = NULL; 	// 로그 파일 FILE 포인터 변수

// add 옵션 구조체 - 옵션 정보 보관용
typedef struct ADD_OPTION {
	char opt_print[10]; // ex) m,n,t,d

	int m_flag; // m옵션이면 1, 아니면 0

	int n_flag; // n옵션이면 1, 아니면 0
	int n_number; // n_number 저장

	int t_flag; // t옵션이면 1, 아니면 0
	int t_time; // t_time 저장

	int d_flag; // d옵션이면 1, 아니면 0
	char d_directory[1024]; // d_directory 저장
} add_option;

// 백업 리스트 - 링크드 리스트로 구현
typedef struct BACKUP_LIST {
	struct BACKUP_LIST *next;
	pthread_t tid;
	char f_filename[242]; 	// 파일의 절대경로, 크기는 255-13 (_YYMMDDHHMMSS부분 제외)
	char p_filename[242];	// 파일 이름, 파일 이름은 [파일이름_시간]으로 작성하며, 이때 파일이름 부분에 해당, 따라서 크기는 최대 255-13
	int period;		// 파일의 백업 주기
	add_option opt_data;	// 옵션 정보 보관
} backup_list;

// 백업 리스트의 헤더와 테일 - 링크드 리스트로 구현
backup_list *HEAD = NULL, *TAIL = NULL;

// 함수 원형
void rpath_to_fpath(char *r_path, char *f_path);
void thread_function(void *arg);
void backup_function(backup_list *data);
void print_log(char *name, int index);
time_t change_time(char *time);
void directory_search(char *directory, backup_list temp);
int search_list(char *filename);
void delete_list(char *filename);
void default_list();
backup_list *make_backup_list(char filename[242], int period, add_option opt_data);
void thread_function(void *arg);
void backup_function(backup_list *data);
void print_log(char *name, int index);

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

// 문자열로 주어진 시간을 time_t 구조체에 할당하는 함수
// 문자열은 YYMMDDHHMMSS로 주어진다.
time_t change_time(char *time) {
	time_t create_time;
	struct tm c_f_time;
	char temp[5];

	// 년
	temp[0] = '2';
	temp[1] = '0';
	temp[2] = time[0];
	temp[3] = time[1];
	temp[4] = '\0';
	c_f_time.tm_year = atoi(temp) - 1900;	// 년도는 1900년부터 시작

	// 월
	temp[0] = time[2];
	temp[1] = time[3];
	temp[2] = '\0';
	c_f_time.tm_mon = atoi(temp) - 1;	// 월은 0부터 시작

	// 일
	temp[0] = time[4];
	temp[1] = time[5];
	temp[2] = '\0';
	c_f_time.tm_mday = atoi(temp);

	// 시간
	temp[0] = time[6];
	temp[1] = time[7];
	temp[2] = '\0';
	c_f_time.tm_hour = atoi(temp);
	
	// 분
	temp[0] = time[8];
	temp[1] = time[9];
	temp[2] = '\0';
	c_f_time.tm_min = atoi(temp);
	
	// 초
	temp[0] = time[10];
	temp[1] = time[11];
	temp[2] = '\0';
	c_f_time.tm_sec = atoi(temp);

	// summer time 사용 안함
	c_f_time.tm_isdst = 0;

	create_time = mktime(&c_f_time);
	return create_time;	
}

// 디렉토리를 탐방하는 함수 - 디렉토리 안의 모든 파일을 백업 리스트에 추가
void directory_search(char *directory, backup_list temp) {
	
#ifdef DEBUG
	printf("디렉토리 탐방 시작!\n");
#endif

	struct dirent **namelist;
	int cont_cnt = scandir(directory, &namelist, NULL, alphasort);

#ifdef DEBUG
	printf("cont_cnt : %d\n", cont_cnt);
#endif

	for (int i = 0; i < cont_cnt; i++) {

		printf("발견된 파일 이름 : %s\n", namelist[i]->d_name);

		// .와 ..는 제외
		if ((!strcmp(namelist[i]->d_name, ".")) || (!(strcmp(namelist[i]->d_name, ".."))))
				continue;

		char resource[1024]; // 파일 크기 제한은 255 + 1('\0')
		struct stat src;
		sprintf(resource, "%s/%s", directory, namelist[i]->d_name);
		stat(resource, &src);

#ifdef DEBUG
		printf("절대경로 : %s\n", resource);
#endif
		

		// 만약 파일 길이 제한 (255 + 1)을 넘을 시 error 처리
		if (strlen(resource) > 255) {
			fprintf(stderr, "%s는 파일 길이 제한(255byte) 을 넘어서 오류!\n", resource);
			continue;
		}

		// 만약 디렉토리라면 또 탐방하면 됨! (재귀호출 시행)
		if (S_ISDIR(src.st_mode)) {
#ifdef DEBUG
			printf("디렉토리를 만났으니 탐방 시작!\n");
#endif
			directory_search(resource, temp);
		}
		else if (S_ISREG(src.st_mode)) {
			// 디렉토리가 아닌 파일 중 일반 파일들은 백업 리스트에 추가하기!
			// 단 백업 리스트에 있으면 안됨!
#ifdef DEBUG

			printf("%s는 일반파일!\n", resource);
#endif

			if (search_list(resource) == -1) {
#ifdef DEBUG
				printf("백업 리스트에 파일이 없으니 추가!\n");
#endif
				add_option temp_this;
				strcpy(temp_this.opt_print, temp.opt_data.opt_print);
				// temp_this.opt_data.opt_print[strlen(temp_this.opt_data.opt_print)-1] = '\0'; 
				// d옵션 지워주기, 지우지 않고, 이 파일은 d옵션에 의해 생성되었다고 나타내어도 될듯
				temp_this.m_flag = temp.opt_data.m_flag;
				temp_this.n_flag = temp.opt_data.n_flag;
				temp_this.n_number = temp.opt_data.n_number;
				temp_this.t_flag = temp.opt_data.t_flag;
				temp_this.t_time = temp.opt_data.t_time;
				temp_this.d_flag = 0;
				temp_this.d_directory[0] = '\0';

				// 백업 리스트에 추가
				if (HEAD == NULL) {
					HEAD = make_backup_list(resource, temp.period, temp_this);
					TAIL = HEAD;
				}
				else {
					TAIL->next = make_backup_list(resource, temp.period, temp_this);
					TAIL = TAIL->next;
				}
			}
			else {
				fprintf(stderr, "%s exist in backup list!\n", resource);
				continue;
			}
		}
		else {
			// 일반 파일이 아님!
			fprintf(stderr, "%s isn't regular file, so don't add this file!\n", resource);
			continue;
		}
	}
}

// 백업 리스트에서 파일을 검색하는 함수, 파일이 있으면 0을 리턴하고, 파일이 없으면 -1을 리턴한다.
int search_list(char *filename) {
	backup_list *cur;
	for (cur = HEAD; cur != NULL; cur = cur->next) {
		if (strcmp(cur->f_filename, filename) == 0)
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
	if (strcmp(HEAD->f_filename, filename) == 0) {
		// 만약 HEAD와 TAIL이 동일할 경우, TAIL은 NULL로... (결과적으로 HEAD도 NULL이 될 거임)
		if (TAIL == HEAD) {
			TAIL = NULL;
		}
		delete_this = cur;
		HEAD = HEAD->next;
		free(delete_this);
		return;
	}

	// HEAD는 지워지지 않음
	for (cur = HEAD; (cur != NULL && cur->next != NULL); cur = cur->next) {
		if (strcmp(cur->next->f_filename, filename) == 0) {
			delete_this = cur->next;
			// TAIL 검사 후 TAIL이 지워지는 거면 갱신
			if (cur->next->next == NULL) {
#ifdef DEBUG
				printf("TAIL이 지워지는 것!\n");
#endif
				TAIL = cur;
				TAIL->next = NULL;
			}
			else {
				// TAIL이 지워지지 않으므로 갱신해줌
				cur->next = cur->next->next;
			}
			free(delete_this);
#ifdef DEBUG
			printf("HEAD는 안지워지고 완료!\n");
#endif
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
		// 지운다는거 로그에 추가
		print_log(cur->f_filename, 3);
		HEAD = HEAD->next;
		free(cur);
	}	
	HEAD = NULL, TAIL = NULL;
}

// 백업 리스트에 새로 추가하는 함수, 쓰레드를 하나 생성하는 함수
backup_list *make_backup_list(char filename[242], int period, add_option opt_data) {
	backup_list *temp = (backup_list *)malloc(sizeof(backup_list));
	// 절대경로 입력
	strcpy(temp->f_filename, filename);

#ifdef DEBUG

	printf("절대경로 : %s\n", filename);
	printf("절대경로 : %s\n", temp->f_filename);

#endif
	// 파일이름 입력(절대경로에서 추출)
	int k;
	for (k = strlen(temp->f_filename) - 1; k >= 0; k--) {
		if (filename[k] == '/')
			break;
	}
	k++;
	char p_filename[300];
	int i;
	for (i = k; i < strlen(temp->f_filename); i++) {
		p_filename[i-k] = temp->f_filename[i];
	}
	p_filename[i-k] = '\0';
	strcpy(temp->p_filename, p_filename);
#ifdef DEBUG
	printf("상대경로 : %s\n", temp->p_filename);
#endif
	

	// period 입력
	temp->period = period;

	// option 입력 (이미 추출됨)
	strcpy(temp->opt_data.opt_print, opt_data.opt_print);
	temp->opt_data.m_flag = opt_data.m_flag;
	temp->opt_data.n_flag = opt_data.n_flag;
	temp->opt_data.n_number = opt_data.n_number;
	temp->opt_data.t_flag = opt_data.t_flag;
	temp->opt_data.t_time = opt_data.t_time;
	temp->opt_data.d_flag = opt_data.d_flag;
	strcpy(temp->opt_data.d_directory, opt_data.d_directory);

	// 다음 가리키는 곳은 NULL로 지정
	temp->next = NULL;

	// add 명령어가 성공했으니, added되었다고 써주자!
	print_log(temp->f_filename, 1);

	// 쓰레드 생성! -> 백업 수행
	if (pthread_create(&(temp->tid), NULL, (void *)(&thread_function), (void *)temp) != 0) {
		fprintf(stderr, "pthread_create error\n");
	}
	return temp;
}

// 새로 생성된 쓰레드가 수행할 함수, 새로 생성된 쓰레드의 시작지점
void thread_function(void *arg) {
	backup_list *data = (backup_list *)arg;
	backup_function(data);
}

// 백업을 수행하는 함수 - generated 할수 있게!!
void backup_function(backup_list *data) {

	// deep copy 수행
	backup_list temp;
	strcpy(temp.f_filename, data->f_filename);
	strcpy(temp.p_filename, data->p_filename);
	temp.period = data->period;
	temp.tid = data->tid;

	strcpy(temp.opt_data.opt_print, data->opt_data.opt_print);
	temp.opt_data.m_flag = data->opt_data.m_flag;
	temp.opt_data.n_flag = data->opt_data.n_flag;
	temp.opt_data.n_number = data->opt_data.n_number;
	temp.opt_data.t_flag = data->opt_data.t_flag;
	temp.opt_data.t_time = data->opt_data.t_time;
	temp.opt_data.d_flag = data->opt_data.d_flag;
	strcpy(temp.opt_data.d_directory, data->opt_data.d_directory);

	// m옵션이 있었다면 파일의 mtime을 미리 갖고와서 저장해 둔다.
	time_t old;
	if (temp.opt_data.m_flag == 1) {
		struct stat statbuf_temp;
		stat(temp.f_filename, &statbuf_temp);
		old = statbuf_temp.st_mtime;
	}

	// n옵션과 t옵션 모두 공통적으로 파일 이름을 보관하는 배열이 필요하니
	// 이거를 만들어서 써먹어 보자
	char name_save[1024][1024];

	// n옵션이 있었다면
	// n_number가 있을 테니 이거는 쓰레드 안에서 변수를 하나 두고,
	// 증가시키다가 n_number에 도달하면 하나씩 삭제하는걸 시작하자 (시간단위로 ㄱㄱ)
	// 여기서는 변수만 정의해 둔다.
	// 점화식을 세워 보자
	int n_count = 0;
	int n_index = 0;

	// t옵션이 있었다면
	// 현재시간 - 시작시간 > time일 경우 그 파일을 삭제하면 된다.
	// 여기서는 변수만 정의해 둔다.
	int t_index = 0;

	// d옵션이 있었다면
	// d_directory를 탐방해서 파일들을 전부 list에 추가해야 한다.
	// 탐방시 . ..는 제외
	// 오름차순 정렬의 경우 recover에서 사용한다.
	// 이때 옵션처리도 해 주어야 하는데 이는 temp.opt_data랑 같게 하면 된다.
	printf("d_flag : %d\n", temp.opt_data.d_flag);
	if (temp.opt_data.d_flag == 1) {
#ifdef DEBUG
		printf("d옵션 처리해주는 함수호출 시작!\n");
#endif
		directory_search(temp.opt_data.d_directory, temp);
	}

	// 반복하여 백업
	while (1) {
		// period 동안 대기
		sleep(temp.period);

		// 백업 리스트를 순회하면서 만약 자기 자신이 없으면
		// 백업 중단하고 빠져나옴 (쓰레드 종료)
		if (search_list(temp.f_filename) == -1) {
			pthread_exit(0);	
		}

		// 만약 m옵션이 있었다면, mtime이 수정되었는지 확인하고 수정되었을 경우만 백업 수행
		if (temp.opt_data.m_flag == 1) {
			// 원본 파일의 mtime이 수정되지 않았다면 continue;
			struct stat statbuf_temp2;
			stat(data->f_filename, &statbuf_temp2);
			if (old == statbuf_temp2.st_mtime)
				continue;
			else
				old = statbuf_temp2.st_mtime;
		}

		// 백업 수행
		// 터미널 명령어 cp 사용
		// cp 원본파일(절대경로 포함) 복사할파일(절대경로 포함)
		// 복사할파일 : [파일이름_시간]
		char command[1024];
		char name[600];
		char name_first[300];
		char name_second[300];
		strcpy(command, "cp ");
		strcat(command, temp.f_filename);
		strcat(command, " ");
		time_t timer = time(NULL);
		struct tm *t = localtime(&timer);
		sprintf(name_first, "%s/%s_", backup_dir, temp.p_filename);
		sprintf(name_second, "%02d%02d%02d%02d%02d%02d", t->tm_year % 100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
#ifdef DEBUG
		printf("백업완료된 파일의 이름\n");
		printf("backup_dir : %s\n", backup_dir);
		printf("temp.p_filename : %s\n", temp.p_filename);
#endif
		strcat(command, name_first);
		strcat(command, name_second);
		system(command);
		strcpy(name, name_first);
		strcat(name, name_second);
		print_log(name, 2);

		// 만약 n옵션과 t옵션이 같이 있는 경우, 둘 다 삭제 기능이 있어서, n옵션에서 파일이 삭제되었다면 t옵션에도 영향을 준다.
		// 따라서, n옵션과 t옵션 둘 중 한 곳에서만 삭제된다면, 다른 곳에서는 볼 필요가 없다.
		// 지금 이 백업파일들은 시간에 따라서 정렬될 수 있으므로, n옵션과 t옵션 중 뭐가 더 센지 확인해 보자.
		// PERIOD는 5 ~ 10이고, -n옵션의 number는 1 ~ 100이고, -t옵션의 TIME은 60 ~ 1200이다.
		// 만약 PERIOD = 5, number = 10이라고 해 보자. 이러면 5초간 백업이 이루어지며 파일은 최대 50초동안 유지되게 될 것이다.
		// 그러나 만약 TIME이 50보다 작은 값이라고 해 보자. 이러면 TIME에 의해 TIME초 이전의 파일들은 지워지게 된다.
		// 따라서 TIME이 50(PERIOD * number)보다 작은 값이라면 -t옵션에 의해 파일이 지워지기 때문에, n옵션은 의미가 없게 된다.
		// 반대로 TIME이 50(PERIOD * number)보다 큰 값이라면 -n옵션에 의해 파일이 지워지기 때문에, t옵션은 의미가 없게 된다.
		//
		// 정리해 보자. number * PERIOD > TIME일 경우는 -t옵션이 지배하게 된다. (n옵션이 의미가 없다.)
		// 반면 number * PERIOD <= TIME일 경우는 -n옵션이 지배하게 된다. (t옵션이 의미가 없다. 등호는 어디에 붙여도 상관없음)

		// n 옵션이 있는 경우 + t 옵션이 없거나 있어도 number * PERIOD <= TIME일 경우
		// name_save와 n_count, n_index 이용
		if (temp.opt_data.n_flag == 1 && (temp.opt_data.t_flag == 0 || temp.period * temp.opt_data.n_number <= temp.opt_data.t_time)) {
#ifdef DEBUG
			printf("n옵션이 우선!\n");
#endif
			
			// 만약 백업한 파일의 개수가 NUMBER개보다 작다면, name_save에 그 이름 등록
			if (n_count < temp.opt_data.n_number) {
				strcpy(name_save[n_count], name_second);
				n_count++;
			}
			else {
				// 백업한 파일의 개수가 NUMBER개일 경우, 맨 처음에 백업한 파일을 지움으로써 백업한 파일이 최대 NUMBER개가 되도록 계속 유지해 준다.
				// rm -f "%s/%s_(name_second)"
				strcpy(command, "rm -f ");
				sprintf(name_first, "%s/%s_", backup_dir, temp.p_filename);
				strcat(command, name_first);
				strcat(command, name_save[n_index]);
#ifdef DEBUG
				printf("%s\n", command);
#endif
				system(command);

				// 최근 백업한 파일은 name_save[n_index]에 저장한다.
				strcpy(name_save[n_index], name_second);
				n_index++;

				// n_index가 NUMBER와 같아지면 0으로 다시 돌아가게 한다.
				if (n_index == temp.opt_data.n_number)
					n_index = 0;
			}
		}

#ifdef DEBUG
		printf("temp.period : %d, temp.opt_data.n_number : %d, temp.opt_data.t_time : %d\n", temp.period, temp.opt_data.n_number, temp.opt_data.t_time);
#endif

		// t 옵션이 있는 경우 + n 옵션이 없거나 있어도 number * PERIOD > TIME일 경우
		if (temp.opt_data.t_flag == 1 && (temp.opt_data.n_flag == 0 || temp.period * temp.opt_data.n_number > temp.opt_data.t_time)) {
#ifdef DEBUG
			printf("t옵션이 우선!\n");
#endif
			// 그렇지 않다면 name_save 배열을 공유하지 않는다.
			// 따라서 독자적으로 name_save 배열을 쓸 수 있다.
			// 그 배열에는 생성 시간이 저장될 것이다. (기존 모든 백업 파일의 생성시간을 이렇게 확인한다.)
			if (t_index == 0) {
				strcpy(name_save[t_index], name_second);
				t_index++;
			}
			else {
				// 현재시간 : timer, 파일 생성 시간 : name_save[i]에 문자열로 저장되어 있음
				// 그거를 변환하자.
				for (int i = 0; i < t_index; i++) {

					// 지워진 파일 부분은 넘긴다.	
					if (name_save[i][0] == '0')
						continue;

					// 현재시간 - 파일 생성 시간 > TIME(파일 보관 시간)일 경우
#ifdef DEBUG
					printf("파일 생성 원본 : %s\n", name_save[i]);
					printf("현재 시간      : %ld\n", timer);
					printf("파일 생성 시간 : %ld\n", change_time(name_save[i]));
					printf("TIME : %d\n", temp.opt_data.t_time);
#endif
					if (difftime(timer, change_time(name_save[i])) > temp.opt_data.t_time) {
						// 그 파일을 지워야 한다.
						// rm -f "%s/%s_(name_second)"
						strcpy(command, "rm -f ");
						sprintf(name_first, "%s/%s_", backup_dir, temp.p_filename);
						strcat(command, name_first);
						strcat(command, name_save[i]);
#ifdef DEBUG
						printf("%s\n", command);
#endif
						system(command);

						// 지운 파일 이름 초기화
						strcpy(name_save[i], "000000000000");
					}
				}

				// 최근 파일 저장한 시간도 저장
				int sw = 0;
				for (int i = 0; i < t_index; i++) {
					if (strcmp(name_save[i], "0") == 0) {
						sw = 1;
						strcpy(name_save[i], name_second);
						break;
					}
				}

				if (sw == 0) {
					strcpy(name_save[t_index], name_second);
					t_index++;
				}
			}
		}



	}
}

// 로그 파일에 출력하는 함수
// 입력 매개변수 : 파일 이름 or 절대경로, 출력번호 (1 : added, 2 : generated, 3 : deleted, 4 : recovered)
void print_log(char *name, int index) {
	time_t timer = time(NULL);
	struct tm *t = localtime(&timer);
	char temp1[20];
	char temp2[20];
	char temp_final[300];
	sprintf(temp1, "[%02d%02d%02d %02d%02d%02d] ", t->tm_year % 100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
//	sprintf(temp2, "_%02d%02d%02d%02d%02d%02d", t->tm_year % 100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	pthread_mutex_lock(&mutex);
	fp_log_file = fopen(backup_log_file, "a+");
	switch(index) {
		case 1: // added
#ifdef DEBUG
		//	printf("%s%s added\n", temp1, name);
#endif
			sprintf(temp_final, "%s%s added\n", temp1, name);
			fwrite(temp_final, 1, sizeof(char)*strlen(temp_final), fp_log_file);
			break;
		case 2: // generated
#ifdef DEBUG
		//	printf("%s%s%s generated\n", temp1, name, temp2);
#endif
			sprintf(temp_final, "%s%s generated\n", temp1, name);
			fwrite(temp_final, 1, sizeof(char)*strlen(temp_final), fp_log_file);
			break;
		case 3: // deleted (remove)
#ifdef DEBUG
		//	printf("%s%s deleted\n", temp1, name);
#endif
			sprintf(temp_final, "%s%s deleted\n", temp1, name);
			fwrite(temp_final, 1, sizeof(char)*strlen(temp_final), fp_log_file);
			break;
		case 4: // recovered
#ifdef DEBUG
		//	printf("%s%s recovered\n", temp1, name);
#endif
			sprintf(temp_final, "%s%s recovered\n", temp1, name);
			fwrite(temp_final, 1, sizeof(char)*strlen(temp_final), fp_log_file);
			break;
	}
	fclose(fp_log_file);
	pthread_mutex_unlock(&mutex);
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
		// 백업 디렉토리
		system("mkdir backup");
		rpath_to_fpath("backup", backup_dir);

		// 로그 파일 생성
		strcpy(backup_log_file, backup_dir);
		strcat(backup_log_file, "/backup_log.txt");
		if ((fp_log_file = fopen(backup_log_file, "w+")) == NULL) {
			printf("파일 열기 오류!\n");	
		}
		fclose(fp_log_file);
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
	if ((lstat(argv[1], &file_info)) < 0 && argc == 2) {
		fprintf(stderr, "stat error - don't find %s (directory)\n", argv[1]);
		exit(1);
	}

	if (argc == 2) {
		if (access(argv[1], F_OK) != 0 || (!S_ISDIR(file_info.st_mode)) || access(argv[1], R_OK) != 0 || access(argv[1], W_OK) != 0 ) {
			fprintf(stderr, "usage : ./ssu_backup <DIR>\n");
			exit(0);
		}

		// 모든 조건에 부합할 경우, 인자로 입력받은 디렉토리 안에 백업 디렉토리 생성, 로그파일 생성
		char temp[255] = {"mkdir "};
		strcat(temp, argv[1]);
		strcat(temp, "/backup");
		system(temp);
		rpath_to_fpath(argv[1], backup_dir);
		strcat(backup_dir, "/backup");
		
		strcpy(backup_log_file, backup_dir);
		strcat(backup_log_file, "/backup_log.txt");
		if ((fp_log_file = fopen(backup_log_file, "w+")) == NULL) {
			printf("파일 열기 오류!\n");	
		}
		fclose(fp_log_file);
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
	
	// 백업 리스트 순회,  백업 리스트에 존재하면 에러처리
	if (search_list(filename) == 0) {
		fprintf(stderr, "%s is already exist in backup_list!\n", token[1]);
		return;
	}

	// [PERIOD] : token[2](문자열) -> period(정수)
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
	d_directory[0] = '\0';

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

			// TIME은 정수형이며 초를 나타냄. 범위는 60 <= TIME <= 1200
			t_time = atoi(token[i]);
			if (!(t_time >= 60 && t_time <= 1200)) {
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

			// DIRECTORY 추가 - 절대경로 사용
			//strcpy(d_directory, token[i]);
			rpath_to_fpath(token[i], d_directory);

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

	// add 옵션 구조체를 채워보자
	add_option opt_data;

	int index = 0;
	if (m_flag == 1) {
		if (index != 0) {
			opt_data.opt_print[index] = ',';
			index++;
		}
		opt_data.opt_print[index] = 'm';
		index++;
	}
	
	if (n_flag == 1) {
		if (index != 0) {
			opt_data.opt_print[index] = ',';
			index++;
		}
		opt_data.opt_print[index] = 'n';
		index++;
	}
	
	if (t_flag == 1) {
		if (index != 0) {
			opt_data.opt_print[index] = ',';
			index++;
		}
		opt_data.opt_print[index] = 't';
		index++;
	}
	
	if (d_flag == 1) {
		if (index != 0) {
			opt_data.opt_print[index] = ',';
			index++;
		}
		opt_data.opt_print[index] = 'd';
		index++;
	}

	opt_data.opt_print[index] = '\0';

	opt_data.m_flag = m_flag;
	opt_data.n_flag = n_flag;
	opt_data.n_number = n_number;
	opt_data.t_flag = t_flag;
	opt_data.t_time = t_time;
	opt_data.d_flag = d_flag;
	strcpy(opt_data.d_directory, d_directory);

	// add하는 함수 연결 - pthread 사용해서 링크드리스트에 add하고 백업 시작!
	
	if (HEAD == NULL) {
		HEAD = make_backup_list(filename, period, opt_data);
		TAIL = HEAD;
	}
	else {
		TAIL->next = make_backup_list(filename, period, opt_data);
		TAIL = TAIL->next;
	}
	
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
#ifdef DEBUG
		printf("fpath : %s\n", fpath);
#endif
		if (search_list(fpath) != 0) {
			fprintf(stderr, "%s isn't exist backup_list!\n", token[1]);
			return;
		}

#ifdef DEBUG
		printf("백업 리스트에 존재하니 삭제!\n");
#endif

		// 백업 리스트에 존재하면 삭제
		delete_list(fpath);

#ifdef DEBUG

		printf("로그파일에 찍기!\n");
#endif


		// 로그파일에 찍어보자!
		print_log(fpath, 3);
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
	// 만약 <FILENAME>이 현재 백업 리스트에 존재하지 않는다면 에러 처리
	// 백업 수행 종료 = 백업 리스트에서 삭제하면 된다. 그러면 알아서 백업이 종료되니깐
	
	
	// 구현해보기!!
	 

	// 만약 변경할 파일에 대한 백업 파일이 존재하지 않으면 에러 처리
	
	
	// 구현해보기!!
	

	// 찍어보자!!
	// token[3]은 절대경로만!!
	print_log(token[3], 4);

	return;
}

// 현재 백업 실행중인 모든 백업 리스트 출력
// 한 줄에 한 개 파일의 절대경로, PERIOD, OPTION 출력
void list()
{
	backup_list *cur;
	printf("--------------------------------------------\n");
	printf("[절대경로],       [PERIOD],         [OPTION]\n");
	printf("--------------------------------------------\n");
	for (cur = HEAD; cur != NULL; cur = cur->next) {
		if (cur->opt_data.opt_print[0] == '\0')
			printf("%s, %d\n", cur->f_filename, cur->period);
		else
			printf("%s, %d, %s\n", cur->f_filename, cur->period, cur->opt_data.opt_print);
	}
	return;
}
