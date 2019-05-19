#include <stdio.h>
#include <stdlib.h>	// exit(0) 사용 가능
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dirent.h>
#include <string.h>	// 문자열 함수 사용 가능
#include <errno.h>
#include <pthread.h>	// -lpthread를 사용하기 위한 헤더 파일
#include <time.h>	
#include <ctype.h>      // isdigit(), isalpha(), isspace() 사용 가능
#include "myparse.h"	// 임의로 만든 h 파일이며 파싱 관련 함수들이 존재함
// 학생들이 제출한 프로그램 실행 결과 채점
// COMPILE WARNING이 발생한 문제는 -0.1점
// COMPILE ERROR가 있는 경우 발생한 문제는 0점
// 프로그램의 실행이 5초 이상 걸릴 경우 0점
#define COMPILE_WARNING -0.1
#define COMPILE_ERROR 0
#define TASK_FIVESEC 0

//#define DEBUG_OPTION
//#define DEBUG_OPTION_C
//#define DEBUG_OPTION_T
#define BUFFER_SIZE 1024
#define NAME_SIZE 64
#define SECOND_TO_MICRO 1000000
//#define DEBUG
//#define DEBUG_S
#define COMPILEDEBUG
#define OPTION_C
#define THREAD_PART
//#define DEBUG_THREAD
//#define NEW_PARSING
//#define NEW_PARSING2

#ifdef THREAD_PART

char sub_thread_end = 'f';
char test_thread_flag = 'p'; // pass, fail
char path_thread[1024];
char path_thread_loc[1024];

void *ssu_thread(void *arg);
// 메인 쓰레드
void *ssu_main_thread(void *arg) {
	sub_thread_end = 'f';
	pthread_t tid2;

	//	printf("메인 쓰레드 시작\n");
	if (pthread_create(&tid2, NULL, ssu_thread, NULL) != 0) {
		fprintf(stderr, "pthread_sub_create error!\n");
		exit(1);
	}

	// 서브 쓰레드 기다리기
	sleep(5);

	// 서브 쓰레드가 죽었는지 확인
	if (sub_thread_end != 'p') {
		//		printf("소요시간이 5초 이상! 0점 처리!");

		// 0점 처리를 한다. (전역변수를 f로 넘김)
		test_thread_flag = 'f';

		/*
		   stdin 파일 디스크립터 할당 -> system ps 입력 ->
		   %s %s %s %s 입력받고, %d %s %s %s 입력받음
		   맨 마지막 %s가 13.stdexe인 경우, %d를 죽임
		   system(kill -9 13.stdexe)
		   파일에서 13.stdexe 확인 
		   */

		int file_copys;
		if ((file_copys = open("test.txt", O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0) {
			fprintf(stderr, "open error!\n");
			exit(1);
		}
		// 5초가 지났을 때임!
		int stdout_copys = dup(1);
		int stderr_copys = dup(2);	
		dup2(file_copys, 1);
		dup2(file_copys, 2);

		system("ps");

		//		dup2(stdout_copys, 1); // 쓰레드 종료 직전에 해야 killed가 안뜸!

		char pids[100];
		char temp[100];

		// 첫줄 통과
		FILE *file_copy_struct;
		if ((file_copy_struct = fopen("test.txt", "r")) < 0) {
			fprintf(stderr, "open error!\n");
			exit(1);
		}

		fscanf(file_copy_struct, "%s %s %s %s", temp, temp, temp, temp);

		// 두번째 줄	
		while(1) {
			//			printf("스캔중.... pid는 %s, temp는 %s\n", pids, temp);
			fscanf(file_copy_struct, "%s %s %s %s", pids, temp, temp, temp);
			if (strcmp(temp, path_thread_loc) == 0) {
				//				printf("찾았다!\n");
				break;
			}
		}

		strcpy(temp, "kill -9 ");
		// 에러 메시지인가? test해볼것! - 위에 stderr까지 했음
		strcat(temp, pids);		
		//		printf("system %s 적용!\n", temp);
		system(temp);

		// 파일 지우기
		fclose(file_copy_struct);
		system("rm test.txt");
		dup2(stdout_copys, 1);
		dup2(stderr_copys, 2);
	}
	//	printf("메인 쓰레드 수행 완료\n");
	return NULL;
}

void *ssu_thread(void *arg) {
	//	printf("서브 쓰레드 시작\n");	
	//      system("STUDENT/20190014/13.stdexe");
	system(path_thread);
	//	printf("서브 쓰레드 수행 완료\n");
	sub_thread_end = 'p';
	pthread_exit(NULL);
	return NULL;
}



#endif


// 두개의 문자열을 스페이스 구분없이 비교하는 함수
// 입력값 : 문자열1, 문자열2, 그리고 문자열을 비교할 크기
int str_n_ns_cmp(char *in, char *des, int size)
{
	char *p;
	char *d;
	int i = 0; 

	for(p = in, i = 0, d = des; *p; p++, i++, d++){

		if(i == (size-1)) return 0;

		if(*p == ' ') {
			d--;
			i--;
			continue;
		}

		if(*p == *d) continue;
		if(*p == (*d ) -('a' - 'A')) continue;
		if((*p) - ('a' - 'A') == *d) continue;
		return i+1;
	}

	return i+1;
}

// begin_t부터 end_t까지의 소요 시간을 ms 단위까지 출력하여 주는 함수
void ssu_runtime(struct timeval* begin_t, struct timeval* end_t) {
	end_t -> tv_sec -= begin_t -> tv_sec;

	if(end_t -> tv_usec < begin_t -> tv_usec) {
		end_t -> tv_sec--;
		end_t -> tv_usec += SECOND_TO_MICRO;
	}

	end_t -> tv_usec -= begin_t -> tv_usec;
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_t -> tv_sec, end_t -> tv_usec);
}

// 디렉토리를 정렬할 때 비교하는 함수
int (*fnPtr)(const struct dirent **e1, const struct dirent **e2);
int mySort(const struct dirent **e1, const struct dirent **e2) {

	if (strcmp((*e1)->d_name, "score_table.csv") == 0) return 1;
	if (strcmp((*e2)->d_name, "score_table.csv") == 0) return -1;

	char a[6], b[6];
	int i = 0;

	strcpy(a, (*e1)->d_name);
	strcpy(b, (*e2)->d_name);
	int temp1_1, temp1_2, temp2_1, temp2_2;

	char *ptr[2];
	char *ptr2[2];
	char *token;
	char *token2;
	char tmp[3][3] = {};
	char tmp2[3][3] = {};

	token = strtok_r(a, "-", &ptr[0]);
	while (token) {
		strcpy(tmp[i], token);	
		token = strtok_r(tmp[i], "-", &ptr[1]);
		while (token) {
			strcpy(tmp[i], token);
			token = strtok_r(NULL, "-", &ptr[1]);
		}
		token = strtok_r(NULL, "-", &ptr[0]);
		i++;
	}

	temp1_1 = atoi(tmp[0]);
	temp1_2 = atoi(tmp[1]);

	i = 0;
	token2 = strtok_r(b, "-", &ptr2[0]);
	while (token2) {
		strcpy(tmp2[i], token2);	
		token2 = strtok_r(tmp2[i], "-", &ptr2[1]);
		while (token2) {
			strcpy(tmp2[i], token2);
			token2 = strtok_r(NULL, "-", &ptr2[1]);
			i++;
		}
		token2 = strtok_r(NULL, "-", &ptr2[0]);
	}

	temp2_1 = atoi(tmp2[0]);
	temp2_2 = atoi(tmp2[1]);

	int result;
	if (temp1_1 < temp2_1)
		result = -1;
	else if (temp1_1 > temp2_1)
		result = 1;
	else if (temp1_2 < temp2_2)
		result = -1;
	else
		result = 1;

	return result;
}

// 점수 구조체
struct score {
	char name[NAME_SIZE];
	double score;
	double all_score;
};

// 인자를 받는 main함수
int main(int argc, char *argv[])
{
	// 변수 선언
	struct timeval begin_t, end_t;
	// 옵션인자가 들어왔음을 나타내는 int형 변수(default: 0, 인자가 들어오면 1)
	int opt_c = 0;
	int opt_h = 0;
	int opt_e = 0;
	int opt_p = 0;
	int opt_t = 0;

	// 가변인자 5개 보관함
	char _c[5][20]; // c 인자 예시 : -c 20190000 20190001 20190002
	char _t[5][3];  // t 인자 예시 : -t 12 15
	char _e[100]; // e 인자 예시 : -e error

	int select_type;
	int i; int j; int j_2 = 0; int t_j_2 = 0;
	extern char *optarg;

	// 프로그램 수행 시간 측정 시작
	gettimeofday(&begin_t, NULL);

#ifdef DEBUG_OPTION

	printf("인자의 개수는 %d개\n", argc);

#endif

	// 예외 처리 - 인자가 들어오지 않을 경우
	switch (argc) {
		case 0:
		case 1:
			fprintf(stderr, "Usage : ssu_score <STUDENTDIR> <TRUEDIR> [option]\n");
			printf("If you need help, please ./ssu_score -h\n");
			exit(1);
			break;
		case 2:
			printf("%s\n", argv[1]);
			// -h 옵션인지 확인, 이 옵션이 아닐 경우 종료
			if (strcmp(argv[1], "-h") == 0) {
				printf("Usage : ssu_score <STUDENTDIR> <TRUEDIR> [OPTION]\n");
				printf("Option :\n");
				printf(" -e <DIRNAME>	print error on 'DIRNAME/ID/qname_error.txt' file\n");
				printf(" -t <QNAMES>	compile QNAME.C with -lpthread option\n");
				printf(" -h		print usage\n");
				printf(" -p		print student's score and total average\n");
				printf(" -c <IDS> 	print ID's score\n");
				gettimeofday(&end_t, NULL);
				ssu_runtime(&begin_t, &end_t);
				exit(0);
			}

			fprintf(stderr, "Usage : ssu_score <STUDENTDIR> <TRUEDIR> [option]\n");
			printf("If you need help, please ./ssu_score -h\n");
			exit(1);
			break;
		case 3:
			// -h 옵션이 있을 경우 사용법을 출력하고 종료
			if (strcmp(argv[1], "-h") == 0 || strcmp(argv[2], "-h") == 0) {

				if (strcmp(argv[i], "-h") == 0) {
					printf("Usage : ssu_score <STUDENTDIR> <TRUEDIR> [OPTION]\n");
					printf("Option :\n");
					printf(" -e <DIRNAME>	print error on 'DIRNAME/ID/qname_error.txt' file\n");
					printf(" -t <QNAMES>	compile QNAME.C with -lpthread option\n");
					printf(" -h		print usage\n");
					printf(" -p		print student's score and total average\n");
					printf(" -c <IDS> 	print ID's score\n");
					gettimeofday(&end_t, NULL);
					ssu_runtime(&begin_t, &end_t);
					exit(0);
				}
			}
			// 인자의 개수가 3개일 때, ./ssu_score -c 20190000 이 꼴은 아닌지
			// 즉 -c의 인자를 1개만 받는 것은 아닌지) 살펴본다.
#ifdef DEBUG_OPTION
			printf("인자의 개수는 3개!\n");
#endif
			if ((strcmp(argv[1], "-c")) == 0) {
				opt_c = 2; // opt_c가 2일 경우, c옵션만 하고 프로그램 종료
#ifdef DEBUG_OPTION

				printf("c옵션 발견!\n");

#endif
				if (argv[2][0] == '-') {
					printf("-c 옵션 바로 뒤에는 다른 옵션이 아닌, 해당 학생의 학번이 존재해야 합니다.\n");
					fprintf(stderr, "Usage : ssu_score <STUDENTDIR> <TRUEDIR> [option]\n");
					printf("If you need help, please ./ssu_score -h\n");
					exit(1);
				}
				else {
#ifdef DEBUG_OPTION	
					printf("인자 3개에서 c 옵션 발견!\n");
#endif
					strcpy(_c[0], argv[2]);
				}
			}
			break;

		default:
			// 인자의 개수가 3개를 넘을 때, 각 옵션들을 살펴본다.
			// 만약 인자가 서로 중복된다면 오류를 내뿜고 종료하자! (예외처리)
			if (argc > 3) {
				for (i = 3; i < argc; i++) {
					// 인자 중 -h옵션이 있는 경우
					// 이 경우는 사용법만 출력하고 프로그램을 종료한다.
					// 이 옵션은 함수화하자!
					if (strcmp(argv[i], "-h") == 0) {
						printf("Usage : ssu_score <STUDENTDIR> <TRUEDIR> [OPTION]\n");
						printf("Option :\n");
						printf(" -e <DIRNAME>	print error on 'DIRNAME/ID/qname_error.txt' file\n");
						printf(" -t <QNAMES>	compile QNAME.C with -lpthread option\n");
						printf(" -h		print usage\n");
						printf(" -p		print student's score and total average\n");
						printf(" -c <IDS> 	print ID's score\n");
						gettimeofday(&end_t, NULL);
						ssu_runtime(&begin_t, &end_t);
						exit(0);
					}
				}

				for (i = 3; i < argc; i++) {
					// 인자 중 -t옵션이 있는 경우 (뒤에 추가로 숫자 인자가 붙는다. 최대 5개까지만 저장해놓자)
					// 이 경우는 아래에서 컴파일 시 -lpthread 옵션을 추가해 준다.
					// system 명령어에서 컴파일할 때 if문으로 구분해서, 저 부분 추가.
					if (strcmp(argv[i], "-t") == 0) {
						opt_t = 1;
						t_j_2 = 0;
						for(j = i+1; j <= i+5 && j < argc; j++) {
							if (argv[j][0] == '-') 		// 옵션인자이면 break
								break;
							else
								strcpy(_t[t_j_2], argv[j]); // -t의 인자이므로 일단 받는다.
							t_j_2++;
							// -t의 인자가 될 수 있는지 여부는 나중에 판단
						}
					}

					// 인자 중 -p옵션이 있는 경우
					// 이 경우는 각 학생의 점수를 출력하고 전체 평균도 출력하면 된다.
					// _p_flag를 하나 만들어서 이게 1이면 밑에 만들면 됨!
					if (strcmp(argv[i], "-p") == 0) {
						opt_p = 1;
					}


					// 인자 중 -e옵션이 있는 경우
					// 이 경우는 컴파일시 에러 메시지가 뜨면 이를 따로 출력해준다.
					// 먼저 DIRNAME 디렉토리를 생성(mkdir)하고
					// 에러가 뜨면 DIRNAME/각 학번 디렉토리를 생성하고
					// 그 안에 문제번호_error.txt를 하나 만든 뒤 거기에 오류를 쓴다.
					// -e옵션은 인자를 한개만 받는다.
					if (strcmp(argv[i], "-e") == 0) {
						opt_e = 1;

						if (i+1 >= argc || argv[i+1][0] == '-')  // 옵션인자이면
							break;
						else {
							strcpy(_e, argv[i+1]); // -e의 인자이므로 일단 받는다.
							// -e의 인자가 될 수 있는지 여부는 나중에 판단
						}
					}

					// 인자 중 -c옵션이 있는 경우 (뒤에 추가로 숫자 인자가 붙는다. 최대 5개까지만 저장해놓자)
					// 이 경우는 채점결과 파일이 있는 경우 해당 학생들의 점수를 출력하면 된다.
					// 채점 결과 파일이 없으면 하지 말고, 있으면 채점 전에 먼저 하면 됨!
					if (strcmp(argv[i], "-c") == 0) {
						opt_c = 1;

						j_2 = 0;
						for(j = i+1; j <= i+5 && j < argc; j++) {
#ifdef DEBUG_OPTION
							printf("j는 %d, argc는 %d\n", j, argc);
							printf("c옵션 시작! %s\n", argv[j]);
#endif
							if (argv[j][0] == '-')			// 옵션인자이면 break
								break;
							else {
#ifdef DEBUG_OPTION
								printf("인자를 받자!\n");
#endif
								strcpy(_c[j_2], argv[j]); // -c의 인자이므로 일단 받는다.
							}
							j_2++;
							// -c의 인자가 될 수 있는지 여부는 나중에 판단
						}


					}
				}
			}

			break;	
	}

	// -e옵션일경우 디렉토리 확인
	if(opt_e == 1) {
		// 존재하지 않으면 그냥 새로 만들어서, 내용을 복사하고 원본은 지우면 된다.
		char _e_new_mkdir[100] = "./";
		strcat(_e_new_mkdir, _e);

		// debug
		// printf("디버그\n");

		// DIRNAME 폴더가 존재하는가? 만약 없다면 새로 생성(최초 1번만 보면 된다.)
		if (access(_e_new_mkdir, 0) == -1) {
			mkdir(_e_new_mkdir, 0777);
		}
		else {
			// 만약 이 폴더가 있다면, 그 안에 있는 data를 모두 지우고 다시 생성해야 함
			//					printf("기존에 폴더가 있습니다! 삭제 시작...\n");
			char _e_rm_mkdir[100] = "rm -r ";
			strcat(_e_rm_mkdir, _e_new_mkdir);
			//system("rm -r _e_new_mkdir");
			system(_e_rm_mkdir);
			//					printf("삭제 완료! 다시 생성 시작...\n");
			mkdir(_e_new_mkdir, 0777);

			// DEBUG, 여기 다시 올 일 없음!	
		}
	}
#ifdef DEBUG_OPTION

	if (opt_c == 2)
		printf("-c 옵션만 있네요!\n");

	if (opt_t == 1)
		printf("-t 발견!\n");

	if (opt_c == 1)
		printf("-c 발견!\n");

	if (opt_e == 1)
		printf("-e 발견!\n");

	for(i = 0; i < 5; i++) {
		printf("_e : %s, _c : %s, _t : %s\n", _e, _c[i], _t[i]);
	}

	printf("-c 옵션이 2일 수 있는데, 그러면 바로 밑에 표시될것임!\n");

	//	exit(0);

#endif

	// 해당 학생의 점수를 저장해 두는 double형 배열
	// -1.11로 초기화한다. 이는 절대 나올 수 없는 점수이므로, 만약 이 점수가 유지된다면
	// 그 학생이 점수 테이블에 없거나, 혹은 -c옵션 가변인자의 개수가 더 적은 것이다.
	// 이 배열은 opt_c == 1일때 사용될 것이다.
	double opt_c_score[5] = {-1.11, -1.11, -1.11, -1.11, -1.11};

	// p 옵션에서, 전체 학생의 총점을 학생수로 나눌 경우 평균이 된다.
	// 학생 수를 나타내는 변수는 밑에서 선언하기로 한다. (student_count)
	// 지금은 p 옵션에서 전체 학생의 총점을 나타낼 변수를 선언하자.
	double p_total_score_for_student = 0;

	// 만약 채점할 디렉토리도 없이 c옵션만 있는 경우, opt_c = 2로 설정하여, c옵션만 수행하고 프로그램을 종료하게 한다.
	if (opt_t == 0 && opt_e == 0 && opt_p == 0) {
		if (strcmp(argv[1], "-c") == 0) {
			opt_c = 2;
			j_2 = 0;
			for (i = 2; i < argc && j_2 < 5; i++) {
				strcpy(_c[j_2], argv[i]);
				j_2++;
			}

#ifdef DEBUG_OPTION_C
			for(i = 0; i < 5; i++) {
				printf("_e : %s, _c : %s, _t : %s\n", _e[i], _c[i], _t[i]);
			}

			printf("c옵션 가변인자는 %d개\n", j_2);
#endif
			FILE *fp_result_tables;
			// 만약 c 옵션만 설정되었는데 채점 테이블이 없다면, 오류를 내고 프로그램 종료
			// 만약 그렇다면, 채점 결과 테이블에서 그 학번을 검색하여, 총점 출력
			if ((fp_result_tables = fopen("score.csv", "r")) == NULL) {
				fprintf(stderr, "기존에 존재하는 채점 결과 테이블이 없습니다!\n");
				exit(1);
			}
			else {
				// 채점 결과 테이블에서 그 학번 검색
				// 없으면 오류
				// 있으면 총점 출력하고 프로그램 종료
#ifdef OPTION_C
				// c옵션 가변인자의 개수 : j_2
				int j_3 = 0;
				for(j_3 = 0; j_3 < j_2; j_3++) {

					fclose(fp_result_tables);
					fopen("score.csv", "r");

					// csv파일에서, 첫 줄 통과하기(문제번호 있는 줄)
					int a_csv =(int)'\n';
					int b_csv =(int)',';
					int read_csv;
#ifdef DEBUG_OPTION_C
					printf("\\n은 %d\n", a_csv);
#endif
					int comma_count_a = 0;
					while((read_csv = fgetc(fp_result_tables)) && (a_csv!=read_csv)) {
						// , 개수 세기
						if (read_csv == b_csv) comma_count_a++;
						//	printf("시행중입니다..");
					}
#ifdef DEBUG_OPTION_C
					printf("콤마의 개수는 %d\n", comma_count_a);
#endif			
					// 학번의 총 개수만큼 반복
					while(1) {

						//	printf("학번찾기!!!\n");

						// csv파일에서, 학번부분만 추출하기(,만나면 나옴)
						char b_csv_stu_num[100];
						char b_csv_stu_num_bak[100];
						int eof_sw_csv = 0;

						i = 0;
						while((b_csv_stu_num[i] = (char)fgetc(fp_result_tables)) && (b_csv!=(int)b_csv_stu_num[i])) {
							if (b_csv_stu_num[i] == EOF) {
								printf("%s's score doesn't exist!\n", b_csv_stu_num_bak);
								eof_sw_csv = 1;
								break;
							}
#ifdef DEBUG_OPTION_C
							printf("i의 값 : %d, 그 값 : %c\n", i, b_csv_stu_num[i]);
#endif
							i++;
						}

						if (eof_sw_csv == 1) break;

						b_csv_stu_num[i] = '\0';
						strcpy(b_csv_stu_num_bak, b_csv_stu_num);
#ifdef DEBUG_OPTION_C
						printf("학번은 : %s\n", b_csv_stu_num);
#endif


						// 학번이 같으면 점수를 출력
						char c_csv_stu_score[100];
						if (strcmp(b_csv_stu_num, _c[j_3]) == 0) {
							i = 0;
							while (i < comma_count_a-1) {
								read_csv = fgetc(fp_result_tables);
								if (read_csv == b_csv) {
#ifdef DEBUG_OPTION_C
									printf("%d번째 콤마 발견!\n", i+1);						
#endif
									i++;
								}
							}
							i = 0;
							while(1) {
								c_csv_stu_score[i] = (char)fgetc(fp_result_tables);
								if(c_csv_stu_score[i] == a_csv) break;
								i++;
							}
							c_csv_stu_score[i] = '\0';
							printf("%s's score :  %s\n", b_csv_stu_num, c_csv_stu_score);
							// while문을 빠져나간다.
							break;
						}
						else {
							// 학번이 다르면, 그 다음 학번부분까지 가기
							// 만약 EOF를 만나면, 학번을 못 찾았다고 말하고 프로그램 종료!
							i = 0;
							while(i < comma_count_a-1) {
								read_csv = fgetc(fp_result_tables);
								if(read_csv == b_csv) {
									i++;
								}
							}
							i = 0;

							while(1) {
								read_csv = fgetc(fp_result_tables);
								if (read_csv == a_csv) {
#ifdef DEBUG_OPTION_C
									printf("%d vs %d, %d번째\n", read_csv, a_csv, i);
#endif
									break;
								}

								// EOF 설정
								if (read_csv == EOF) {
									printf("%s's score doesn't exist!\n", b_csv_stu_num);
									eof_sw_csv = 1;
									break;	
								}
							}
						}
					}
#endif
					// 디버깅용
					// exit(0);
				}

				// -c 가변 인자에 대해 탐색이 모두 끝나면, 프로그램 종료
				exit(0);
			}


		}
	}



	// 디렉토리의 파일 및 목록을 출력하기
	DIR *dir_ptr = NULL;
	DIR *dir_stu = NULL;
	struct dirent **namelist;
	struct dirent **studentlist;
	fnPtr = &mySort;
	int count;
	int student_count;
	int idx;
	char student_path[1024];
	student_path[0] = '\0';
	char name[1024];
	name[0] = '\0';

	//--------------------------------------------------------------------------------//

	// 학생답안 경로 읽기
	// 만약 입력으로 절대경로가 들어온다면
	if (argv[1][0] == '/') {
		strcat(student_path, argv[1]);
	}
	else {
		// 입력으로 상대경로가 들어온다면
		strcat(student_path, "./");
		strcat(student_path, argv[1]);
	}

	// 목록을 읽을 디렉토리명으로 DIR *를 return 받는다
	if ((dir_stu = opendir(student_path)) == NULL) {
		fprintf(stderr, "%s directory 정보를 읽을 수 없습니다.\n", student_path);
		exit(1);
	}

	//	printf("%d\n", student_count);
	// 디렉토리의 처음부터 파일 또는 디렉토리명을 순서대로 한개씩 읽는다.
	if ((student_count = scandir(student_path, &studentlist, NULL, alphasort)) == -1) {
		fprintf(stderr, "%s Directory Scan Error: %s\n", student_path, strerror(errno));
		exit(1);
	}

	//	printf("%d\n", student_count);

#ifdef DEBUG
	for (idx = 2; idx < student_count; idx++) {
		printf("학생들 : %s\n", studentlist[idx]->d_name);
	}
#endif

	//--------------------------------------------------------------------------------//

	// 정답지 경로 읽기
	// 만약 입력으로 절대경로가 들어온다면
	if (argv[2][0] == '/') {
		strcat(name, argv[2]);
	}
	else {
		// 입력으로 상대경로가 들어온다면
		strcat(name, "./");
		strcat(name, argv[2]);
	}

	// 목록을 읽을 디렉토리명으로 DIR *를 return 받는다
	if ((dir_ptr = opendir(name)) == NULL) {
		fprintf(stderr, "%s directory 정보를 읽을 수 없습니다.\n", name);
		exit(1);
	}


	//	printf("a:%d\n", count);
	// 디렉토리의 처음부터 파일 또는 디렉토리명을 순서대로 한개씩 읽는다.
	if ((count = scandir(name, &namelist, NULL, fnPtr)) == -1) {
		fprintf(stderr, "%s Directory Scan Error: %s\n", name, strerror(errno));
		exit(1);
	}

	//	printf("b:%d\n", count);
#ifdef DEBUG
	for (idx = 2; idx < count; idx++) {
		printf("시작 : %s\n", namelist[idx]->d_name);
	}
#endif
	// txt파일과 c파일의 구분
	struct score score_table[100];
	char name_temp[1024];
	char filename[1024];
	struct dirent *file = NULL;
	DIR *dir_ans = NULL;
	struct stat buf_stat;
	char *ext;

	for(idx = 2; idx < count; idx++) {
		name_temp[0] = '\0';
		strcat(name_temp, name);

		strcat(name_temp, "/");
		strcat(name_temp, namelist[idx]->d_name);

		if ((dir_ans = opendir(name_temp)) == NULL) {
			if(strstr(name_temp, "score_table.csv") != NULL) {
#ifdef DEBUG_S
				printf("score_table.csv 파일이 있습니다!\n");
#endif
				count--;
				break;				
			}
			else {
				fprintf(stderr, "%s directory 정보를 읽을 수 없습니다.\n", name_temp);
				exit(1);
			}
		}

		while((file = readdir(dir_ans)) != NULL) {
			if (strcmp(file->d_name, ".") == 0) {
				//				printf("파일명이 .인 경우 skip\n");
				continue;	
			}

			sprintf(filename, "%s/%s", name_temp, file->d_name);

			if (stat(filename, &buf_stat) == -1) {
				//				printf("파일의 속성을 알아보자\n"); 
				continue;
			}
			else if(S_ISREG(buf_stat.st_mode)) {
				if ((ext = strrchr(filename, '.')) == NULL) continue;
				if (strcmp(ext, ".txt") == 0) {
					//					printf("txt파일 발견!\n");
					strcpy(score_table[idx-2].name, file->d_name);
					score_table[idx-2].all_score = 1; // txt파일은 1로 설정
				} else if (strcmp(ext, ".c") == 0) {
					//.					printf("c파일 발견!\n");
					strcpy(score_table[idx-2].name, file->d_name);
					score_table[idx-2].all_score = 2; // c파일은 2로 설정
				}
			}
			//			printf("왜 파일이 없냐?\n");
		}

		closedir(dir_ans);
	}
#ifdef DEBUG
	for(idx = 0; idx < count - 2; idx++) {
		printf("%s\n", score_table[idx].name);
	}
#endif
	/*
	// ./STD 경로가 있는지를 확인해서, 경로가 없을 경우 새로 만든다.
	DIR *std_directory = NULL;
	int nResult;

	nResult = access("./STD", 0);
	if (nResult == -1) {	
	mkdir("./STD", 0777);
	}
	*/				
	// ./ANS 경로가 있는지를 확인해서, 경로가 없을 경우 새로 만든다.
	FILE *fp;
	int type;
	double blank_score;
	double program_score;
	double question_score;

	//	nResult = access("./ANS", 0);
	//	if (nResult == -1) {
	//		mkdir("./ANS", 0777);
	//	}

	// score_table.csv 파일이 있는지 확인해서, 없을 경우 새로 만든다.
	// 그리고 그 파일에 들어갈 값들을 점수 구조체에도 보관한다.

	char score_table_path[1024];
	score_table_path[0] = '\0';
	strcat(score_table_path, argv[2]);
	strcat(score_table_path, "/score_table.csv");

	if ((fp = fopen(score_table_path, "r+")) == NULL) {
		printf("score_table.csv file doesn't exist!\n");
		fp = fopen(score_table_path, "w+");

		printf("1. input blank question and program question's score. ex) 0.5 1\n");
		printf("2. input all question's score. ex) Input value of 1-1: 0.1\n");
		printf("select type >> ");
		scanf("%d", &type);

		switch (type) {
			case 1:
				printf("Input value of blank question : ");
				scanf("%lf", &blank_score);
				printf("Input value of program question : ");
				scanf("%lf", &program_score);

				for( i = 0; i < count - 2; i++ ) {
					if (score_table[i].all_score == 1) {
						fprintf(fp, "%s, %lf\n", score_table[i].name, blank_score);
						score_table[i].score = blank_score;
					}
					else if (score_table[i].all_score == 2) {
						fprintf(fp, "%s, %lf\n", score_table[i].name, program_score);
						score_table[i].score = program_score;
					}
				}

				break;
			case 2:
				for( i = 0; i < count - 2; i++ ) {
					printf("Input value of %s: ", score_table[i].name);
					scanf("%lf", &question_score);
					fprintf(fp, "%s, %lf\n", score_table[i].name, question_score);
					score_table[i].score = question_score;
				}
				break;
			default:
				printf("잘못 입력하셨습니다! 프로그램을 종료합니다!\n");
				exit(1);
				break;
		}
	}
	else {
		i = 0;
		int comma_position;
		// 만약 csv 파일이 있다면, 그 파일을 읽는다.
		while((fscanf(fp, "%s %lf", score_table[i].name, &score_table[i].score)) != -1) {
			//			printf("%s 문제의 점수는 %lf\n", score_table[i].name, score_table[i].score);
			i++;
			//			printf("오류가 여기에서 나는건가? i는 %d\n", i);
		}

		for(i = 0; i < count - 2; i++) {
			comma_position = strlen(score_table[i].name);
			//printf("%s\n", score_table[i].name);
			score_table[i].name[comma_position-1] = '\0';
#ifdef DEBUG_S
			printf("%s 문제의 점수는 %lf\n", score_table[i].name, score_table[i].score);
#endif
		}
	}

	// ---------------------------------------------------------------------------------//
	//                                 채점 결과 테이블 생성
	// ---------------------------------------------------------------------------------//


	// 채점 결과 테이블 생성
	// 만약 기존에 채점 테이블이 존재한다면, 새 채점 결과 테이블을 만들고 덮어쓰기
	FILE *fp_result_table;

	if ((fp_result_table = fopen("score.csv", "w+")) == NULL) {
		fprintf(stderr, "채점 결과 테이블이 제대로 생성되지 않았습니다!\n");
		exit(1);
	}

	// 채점 결과 테이블의 가로 첫줄 채우기 (빈칸, 문제번호, 문제번호, ... , 문제번호, 총점)
	int fp_result_count;
	fprintf(fp_result_table, " ,");
	for(fp_result_count = 0; fp_result_count < count - 2; fp_result_count++) {
#ifdef DEBUG_S
		printf("시작! fp_result_count = %d\n",  fp_result_count);
#endif
		fprintf(fp_result_table, "%s,", score_table[fp_result_count].name);
	}
	fprintf(fp_result_table, "sum\n");


	// 두번째 줄부터는 다음과 같음
	// (학번, 첫 문제 채점 점수, 두번째 문제 채점 점수, ... , n번째 문제 채점 점수, 총점)
	// 이는 아래 빈칸 채점과 프로그램 채점 부분에서 csv 파일에 쓸 예정

	// ---------------------------------------------------------------------------------//
	//                          빈칸채점 / 프로그램 문제 채점 시작
	// ---------------------------------------------------------------------------------//

	printf("grading student's test papers..\n");

	//	FILE *fp_answer;
	//	FILE *fp_student;
	char trueset_answer[1024];
	//	char trueset_answer_temp[1024];
	char student_answer[1024];
	//	char student_answer_temp[1024];
	int answer_compile_flag = 0;
	double sum_result = 0; // 총점
	int k;
	int error_file_exist = 0;
	// 학생 수에 따라 loop
	for(k = 2; k < student_count; k++) {

		// score.csv 파일에 학번 입력
		fprintf(fp_result_table, "%s", studentlist[k]->d_name);
		sum_result = 0;
#ifdef DEBUG
		printf("\n\n %d번째 학생 \n\n", k - 1);
#endif
		// 문항 수에 따라 loop
		for(i = 2; i < count; i++) {
#ifdef DEBUG
			printf("\n\nk는 %d, i는 %d\n\n", k, i);
#endif
			FILE *fp_answer;
			FILE *fp_student;			
			// 정답파일 불러오기
			char answer_path[1024];
			answer_path[0] = '\0';
			strcat(answer_path, name);
			strcat(answer_path, "/");
			strcat(answer_path, namelist[i]->d_name);
			strcat(answer_path, "/");
			strcat(answer_path, score_table[i-2].name);
#ifdef DEBUG	
			printf("정답파일 출처 : %s\n", answer_path);
#endif
			// 정답파일을 불러올 수 없는 경우, 그 문제는 없는 것
			if ((fp_answer = fopen(answer_path, "r")) == NULL) {
				fprintf(stderr, "정답파일 중 %s file doesn't exist! 프로그램 종료!\n", answer_path);
				exit(1);
			}
			else {
				// 정답파일을 불러왔다면, trueset_answer을 초기화한 다음,  trueset_answer에 쓰기
				trueset_answer[0] = '\0';
				fscanf(fp_answer, "%[^\n]", trueset_answer);
#ifdef DEBUG
				printf("정답파일 : %s\n", trueset_answer);
#endif
			}
			// 학생답안 불러오기
			char student_answer_path[1024];
			student_answer_path[0] = '\0';
			strcat(student_answer_path, student_path);
			strcat(student_answer_path, "/");
			strcat(student_answer_path, studentlist[k]->d_name);
			strcat(student_answer_path, "/");
			strcat(student_answer_path, score_table[i-2].name);
#ifdef DEBUG
			printf("학생파일 출처 : %s\n", student_answer_path);
#endif
			// 학생파일을 불러올 수 없는 경우, 잘못된 답안으로 처리
			if ((fp_student = fopen(student_answer_path, "r+")) == NULL) {
#ifdef DEBUG_S
				printf("학생파일 중 %s file doesn't exist!\n", student_answer_path);
#endif
				fprintf(fp_result_table, ",0");
				goto THISEND;
			} 	
			else {
				// 학생파일을 불러왔다면, student_answer을 초기화한 다음, student_answer에 쓰기
				student_answer[0] = '\0';
				fscanf(fp_student, "%[^\n]", student_answer);
#ifdef DEBUG
				printf("학생파일 : %s\n\n\n", student_answer);
#endif
			}
			// -------------------------------------------------------------------------------- //
			//                         빈칸 문제  채점 알고리즘 적용                                    
			// -------------------------------------------------------------------------------- //

			// 만약 빈칸 문제(txt)가 아니라면, 프로그램 컴파일 문제(c)로 가야 함
			// 우선은, 빈칸 문제가 아닐 시 continue 사용하겠음.
			if (strcmp(strrchr(answer_path, '.'), ".txt") != 0) {

				// -------------------------------------------------------------------------------- //
				//                        컴파일 문제 채점 알고리즘 적용
				// -------------------------------------------------------------------------------- //

				// 학생이 작성해서 답안으로 제출한 프로그램을 컴파일 후
				// 실행파일은 "./argv[1]/학번/문제번호.stdexe" 이름으로 저장

				// 학번 : studentlist[k]->d_name (20150413)
				// 문제 : score_table[i-2].name (11.c)

				char str_path_problem[1024];
				str_path_problem[0] = '\0';
				strcat(str_path_problem, score_table[i-2].name);

				// 문제번호 : str_path_token (11)

				char *str_path_token = strtok(str_path_problem, ".");

				char str_path[1024];
				char str_path_stdout[1024];
				str_path[0] = '\0';
				strcat(str_path, argv[1]);
				strcat(str_path, "/");
				strcat(str_path, studentlist[k]->d_name); // 학번
				strcat(str_path, "/");
				strcat(str_path, str_path_token); // 문제번호
				strcpy(str_path_stdout, str_path); // stdout 하기 위한 경로
				strcat(str_path, ".stdexe");

				// system("gcc ./argv[1]/학번/문제번호.c -o ./argv[1]/학번/문제번호.stdexe")
				char str_system[1024] = "gcc ";

				strcat(str_system, argv[1]);
				strcat(str_system, "/");
				strcat(str_system, studentlist[k]->d_name);
				strcat(str_system, "/");
				strcat(str_system, score_table[i-2].name);
				strcat(str_system, " -o ");
				strcat(str_system, str_path);

				// 만약 -t 옵션이고, -t옵션인자로 받은 문제라면, 컴파일시 -lpthread 옵션을 추가해야 한다.
				// system("gcc ./argv[1]/학번/문제번호.c -o ./argv[1]/학번/문제번호.stdexe -lpthread");

				if (opt_t == 1) {
					for(idx = 0; idx < t_j_2; idx++) {

						// -t 옵션 가변인자와 문제번호가 같으면, -lpthread 옵션 추가
						if(strcmp(_t[idx], str_path_token) == 0) {

							strcat(str_system, " -lpthread");
#ifdef DEBUG_OPTION_T
							printf("%s 문제는 -t 옵션인자 적용된 문제!\n", _t[idx]);
							printf("컴파일 경로 : %s\n", str_system);
#endif
						}
					}
				}

				// debug
				// system(str_system);

				//				printf("%s\n", str_system);

				//******************************************************************.....//
				// 표준 에러 리다이렉션
				//************************************************************************//
				int stdin_copy = dup(0);
				int stdout_copy = dup(1);
				int stderr_copy = dup(2);

				// DIRNAME = e 옵션 인자
				// 우선 오류 메시지를 출력할 장소를 DIRNAME으로 설정하자(argv[1]로 설정)
				// 우선 채점을 위해 DIRNAME/학번/문제번호_error.txt에 에러 메시지를 출력하자.
				// 채점을 하면서, DIRNAME/학번/문제번호_error.txt가 깨끗하면 이를 지우자(-e 옵션에 상관없이)
				// 그리고, 이 파일들은 -e 옵션이 있을 경우 그 위치로 넘기고, 그렇지 않은 경우는 삭제하자.

				int fd_error;
				char fd_error_path[1024];
				fd_error_path[0] = '\0';
				strcat(fd_error_path, argv[1]); // DIRNAME, 우선 argv[1]로 설정
				strcat(fd_error_path, "/");
				strcat(fd_error_path, studentlist[k]->d_name); // 학번
				strcat(fd_error_path, "/");
				strcat(fd_error_path, str_path_token); // 문제번호
				strcat(fd_error_path, "_error.txt");

				// 생성시 오류가 날 경우 종료, error.txt가 기존에 존재할 경우 덮어쓰기(O_TRUNC)
				if ((fd_error = open(fd_error_path, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0) {
					printf("ssu_error.txt creat error!\n");
					exit(1);
				}

				dup2(fd_error, 2);
				system(str_system);

				//				printf("학생파일 컴파일 완료!");

				// stderr 재할당
				dup2(stderr_copy, 2);
				close(fd_error);

				// 학생이 작성해서 답안으로 제출한 프로그램의 실행파일인
				// "./argv[1]/학번/문제번호.stdexe"를 자동으로 실행시키고, (str_path)
				// 실행결과를 "./argv[1]/학번/문제번호.stdout"으로 저장 (str_path_stdout)	
				// 만약 .stdexe 파일이 없다면, 컴파일이 안된 것이므로 0점 처리

				if (access(str_path, 0) != -1) {
					int fd_stdout;
					strcat(str_path_stdout, ".stdout");

					if ((fd_stdout = open(str_path_stdout, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0) {
						printf("fd_stdout creat error!\n");
					}

					time_t first, second;
#ifdef DEBUG_THREAD
					printf("%s 실행 시작!\n", str_path);
#endif
					// 표준 출력 리다이렉션 // 표준 에러 리다이렉션	
					dup2(fd_stdout, 1);
					dup2(fd_stdout, 2);
					first = time(NULL);

#ifdef THREAD_PART
					pthread_t tid;
					strcpy(path_thread, str_path);
					strcpy(path_thread_loc,str_path_token);
					strcat(path_thread_loc,".stdexe");

					//		printf("path_thread_loc : %s\n", path_thread_loc);

					if (pthread_create(&tid, NULL, ssu_main_thread, NULL) != 0) {
						fprintf(stderr, "pthread_create error!\n");
						exit(1);
					}

					sleep(10);

					//		printf("삭제 완료! 종료 예정!\n");
					system("exit");
					//		printf("쓰레드가 완료되기전 main함수가 먼저 종료되면 실행중 쓰레드 소멸\n");

#endif


					//					system(str_path); // 무한루프의 문제가 있음, thread를 사용해서 따로 돌리면 됨. 354p, 362p 참고
					second = time(NULL);
					// stdout 재할당 // stderr 재할당
					dup2(stdout_copy, 1);
					dup2(stderr_copy, 2);
					close(fd_stdout);
#ifdef DEBUG_THREAD
					printf("%s 소요시간 : %f sec\n", str_path, difftime(second, first));
#endif
				}
				else {
#ifdef DEBUG_THREAD
					printf("%s 파일이 없습니다!\n", str_path);	
#endif
				}

				////////////////////////////////////////////////////////////////////////////

				// 정답 파일 컴파일 개선법
				// 정답 파일은 맨 처음에만 컴파일하고, 두번째부터는 하지 않는 것! (구현 성공)

				///////////////////////////////////////////////////////////////////////////

				char str_answer_number_stdout[1024];
				if (answer_compile_flag == 0) {

					// 정답 프로그램의 경우, -lpthread가 없을 시 stderr이 나오는데, 이걸 안나오게 해 주자.
					// answer_error.txt를 하나 만들고 쓴 뒤 바로 지우자. 만약 못만들면 error! (근데 이걸로 에러나면 감점..)
					// 정답 파일은 꼭 다시 지워야 함! 
					int fd_error_answer;
					if ((fd_error_answer = open("answer_error.txt", O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0) {
						printf("answer_error.txt creat error!\n");
						exit(1);
					}

					dup2(fd_error_answer, 2);

					// 정답 프로그램의 실행 파일은 "./argv[2]/문제번호/문제번호.exe" 이름으로 생성

					char str_answer_number[1024];
					//	char str_answer_number_stdout[1024];
					str_answer_number[0] = '\0';
					strcat(str_answer_number, argv[2]);	
					strcat(str_answer_number, "/");
					strcat(str_answer_number, str_path_token);	
					strcat(str_answer_number, "/");
					strcat(str_answer_number, str_path_token);
					strcpy(str_answer_number_stdout, str_answer_number);
					strcat(str_answer_number, ".exe");
					strcat(str_answer_number_stdout, ".stdout");

					// system("gcc ./argv[2]/문제번호/11.c -o ./argv[2]/문제번호/문제번호.exe")
					str_system[0] = '\0';
					strcat(str_system, "gcc ./");
					strcat(str_system, argv[2]);
					strcat(str_system, "/");
					strcat(str_system, str_path_token);
					strcat(str_system, "/");
					strcat(str_system, score_table[i-2].name);
					strcat(str_system, " -o ");
					strcat(str_system, str_answer_number);

					// 만약 -t 옵션이고, -t옵션인자로 받은 문제라면, 컴파일시 -lpthread 옵션을 추가해야 한다.
					// system("gcc ./argv[1]/학번/문제번호.c -o ./argv[1]/학번/문제번호.stdexe -lpthread");

					if (opt_t == 1) {
						for(idx = 0; idx < t_j_2; idx++) {

							// -t 옵션 가변인자와 문제번호가 같으면, -lpthread 옵션 추가
							if(strcmp(_t[idx], str_path_token) == 0) {
#ifdef DEBUG_OPTION
								printf("%s 문제는 -t 옵션인자 적용된 문제!\n", _t[idx]);
#endif
								strcat(str_system, " -lpthread");
								//		printf("%s\n", str_system);
								//		exit(0);
							}
						}
					}

					// system 함수를 사용하여 compile
					system(str_system);

					// stderr 다시 재할당, close하기, answer.txt 삭제
					dup2(stderr_copy, 2);
					close(fd_error_answer);
					remove("answer_error.txt");

					// 각 문제에 대한 정답 프로그램인 "./argv[2]/문제번호/문제번호.c"의 실행 파일인
					// "./argv[2]/문제번호/문제번호.exe"를 실행시켜 (str_answer_number)
					// 실행결과를 "./argv[2]/문제번호/문제번호.stdout"으로 저장 (str_answer_number_stdout)

					// stdout
					if (access(str_answer_number, 0) != -1) {
						int fd_stdout;
#ifdef DEBUG_OPTION
						printf("%s 실행 시작!\n", str_answer_number);
#endif

						if ((fd_stdout = open(str_answer_number_stdout, O_RDWR | O_CREAT | O_TRUNC, 0777)) < 0) {
#ifdef DEBUG_OPTION
							printf("%s 실행 파일 생성 불가!\n", str_answer_number_stdout);
#endif
							exit(1);
						}

						// 표준 출력 리다이렉션	
						dup2(fd_stdout, 1);
						system(str_answer_number);

						// stdout 재할당
						dup2(stdout_copy, 1);
						close(fd_stdout);
					}
					else {
#ifdef DEBUG_OPTION
						printf("%s 정답 파일이 없습니다!\n", str_answer_number);
#endif
					}
				}


				// 프로그램 채점 점수 보관하는 변수	
				double compile_score_csv = 0;
				// 정답 파일과 학생 파일을 비교해 본다.
				// 우선, 오류가 있는지부터 체크한다.
				int compile_error_flag_2 = 0;
				int compile_warning_flag_2 = 0;

				FILE *fd_error_path_csv;
				if ((fd_error_path_csv = fopen(fd_error_path, "r")) == NULL) {
					fprintf(stderr, "error.txt 파일이 없네요!\n");
					exit(1);
				}

				char strTemp_csv[1024];
				char *pStr_csv;

				fseek(fd_error_path_csv, 0, SEEK_SET);

				// 파일의 크기가 0이면 의미 없음
				struct stat statbuf;
				if ((stat(fd_error_path, &statbuf)) < 0) {
					fprintf(stderr, "stat error\n");
					exit(1);
				}

				if (statbuf.st_size == 0) {
#ifdef DEBUG_THREAD
					printf("파일의 크기는 0\n");
#endif
				}
				else {
					// 파일의 크기가 0이 아니라면, 에러의 여부를 확인
					// 분명히, error 또는 warning이 있음

					while (fgets(strTemp_csv, 1024, fd_error_path_csv)) {
						// DEBUG
						// printf("%s\n", strTemp_csv);

						if (strstr(strTemp_csv, "error") != NULL) {
#ifdef DEBUG_THREAD
							printf("error!\n");
#endif
							compile_score_csv = COMPILE_ERROR;
							compile_error_flag_2 = 1;
							break;
						}
						else if (strstr(strTemp_csv, "warning") != NULL && compile_warning_flag_2 == 0) {
#ifdef DEBUG_THREAD
							printf("warning!\n");
#endif
							compile_score_csv += COMPILE_WARNING;
							compile_warning_flag_2 = 1;
						}
					}
					// debug
					// exit(0);
				}

				fclose(fd_error_path_csv);

				// 만약 파일 안에 아무것도 없으면, 또는 e옵션이 아니면 파일은 지운다.
				if (opt_e == 0 || statbuf.st_size == 0) {
					remove(fd_error_path);
				}
				else {
					// 만약 e옵션이면, 이 파일을 옮긴다.
					// 우선 -e옵션으로 받은 인자가 존재하는지 보고, 존재하면 지운다.
					// _e/학번/문제번호_error.txt
					char _e_option_path[1024];
					char _e_option_path_student[1024];
					strcpy(_e_option_path, _e);
					strcat(_e_option_path, "/");
					strcat(_e_option_path, studentlist[k]->d_name);
					strcpy(_e_option_path_student, _e_option_path);
					strcat(_e_option_path, "/");
					strcat(_e_option_path, str_path_token);
					strcat(_e_option_path, "_error.txt");

					// 학번 폴더가 존재하는가? 만약 없다면 새로 생성, 있으면 상관없음
					if (access(_e_option_path_student, 0) == -1) {
						mkdir(_e_option_path_student, 0777);
					}

					// 에러 메시지 파일 생성
					int _e_option_path_fd;
					if ((_e_option_path_fd = open(_e_option_path, O_RDWR | O_CREAT | O_TRUNC, 0666)) < 0) {
						printf("에러 메시지 생성 에러!\n");
						exit(1);
					}

					// 파일 복사
					char mv_function_error_path[500] = "mv ";
					strcat(mv_function_error_path, fd_error_path);
					strcat(mv_function_error_path, " ");
					strcat(mv_function_error_path, _e_option_path);
					system(mv_function_error_path);

					// system("mv fd_error_path _e_option_path");

				}

				if (compile_error_flag_2 == 0 && compile_warning_flag_2 == 1) {
					printf("warning만 있음!\n");
					exit(1);
				}

				// compile_error_flag_2 == 0이면 컴파일에서 에러는 문제가 없었다는 것이다.
				if (compile_error_flag_2 == 0 ) {

					// 정답 실행 파일 불러오기, 불러올 수 없을 경우 0점

					char str_answer_number[1024];
					str_answer_number[0] = '\0';
					strcat(str_answer_number, argv[2]);
					strcat(str_answer_number, "/");
					strcat(str_answer_number, str_path_token);
					strcat(str_answer_number, "/");
					strcat(str_answer_number, str_path_token);
					strcpy(str_answer_number_stdout, str_answer_number);
					strcat(str_answer_number_stdout, ".stdout");

					FILE *answer_exe_file;
					if((answer_exe_file = fopen(str_answer_number_stdout, "r")) == NULL) {
						printf("%s 정답 파일을 불러올 수 없습니다! 0점 처리합니다!\n", str_answer_number_stdout);
						compile_score_csv = 0;
					}
					else {
#ifdef DEBUG_THREAD
						printf("이번에 채점할 정답파일은! %s\n\n\n", str_answer_number_stdout);
#endif
						int read_answer_exe_file_stdout;
						// 정답 실행 파일에 있는 거 char형 변수에 할당
						char answer_exe_file_stdout[1024];
						answer_exe_file_stdout[0] = '\0';
						int i_ans_length = 0;
						while((read_answer_exe_file_stdout = fgetc(answer_exe_file)) && (read_answer_exe_file_stdout != -1)) {

							// 만약 공백이면 할당받지 않음, 공백이 아니면 할당받음
							if (read_answer_exe_file_stdout == (int)' ') continue;
							else {
								answer_exe_file_stdout[i_ans_length] = read_answer_exe_file_stdout;
								i_ans_length++;
							}

						}

						answer_exe_file_stdout[i_ans_length] = '\0';
#ifdef DEBUG_THREAD
						// debug
						printf("정답!!! : %s\n", answer_exe_file_stdout);
						printf("정답파일 길이 : %d\n", i_ans_length);
						// exit(0);
#endif
						// 학생 실행 파일 불러오기, 불러올 수 없을 경우 에러이므로 프로그램 종료
						FILE *student_ans_exe_file;
#ifdef DEBUG_THREAD
						printf("이번에 채점할 학생파일은 : %s\n", str_path_stdout);
#endif
						if ((student_ans_exe_file = fopen(str_path_stdout, "r")) == NULL) {
							fprintf(stderr, "%s 학생 실행 파일을 불러올 수 없는 것은 error! 프로그램 종료\n", str_path_stdout);
							exit(1);
						}

						// 학생 실행 파일에 있는 거 char형 변수에 할당
						int read_stu_answer_exe_file_stdout;
						char stu_answer_exe_file_stdout[10000];
						stu_answer_exe_file_stdout[0] = '\0';
						int i_std_length = 0;
						while((read_stu_answer_exe_file_stdout = fgetc(student_ans_exe_file)) && (read_stu_answer_exe_file_stdout != -1)) {

							// 만약 공백이면 할당받지 않음, 공백이 아니면 할당받음
							if (read_stu_answer_exe_file_stdout == (int)' ') continue;
							else {
								stu_answer_exe_file_stdout[i_std_length] = read_stu_answer_exe_file_stdout;
								i_std_length++;
							}
						}

						stu_answer_exe_file_stdout[i_std_length] = '\0';
#ifdef DEBUG_THREAD
						// debug
						printf("학생!!! : %s\n", stu_answer_exe_file_stdout);
						printf("학생파일 길이 : %d\n", i_std_length);
						//	exit(0);
#endif
						// 정답 파일과 학생 실행 파일 비교
						// 정답파일 길이 == 학생파일 길이일 경우 함수호출하여 비교
						if (i_ans_length == i_std_length) {
							// 같으면 점수를 할당, 다르면 점수 없음
							if(str_n_ns_cmp(answer_exe_file_stdout, stu_answer_exe_file_stdout, i_ans_length) == 0) {
#ifdef DEBUG_THREAD
								printf("이번문제는 정답!\n");
#endif
								compile_score_csv += score_table[i-2].score;
							}
							else {
#ifdef DEBUG_THREAD
								printf("이번문제는 오답!\n");
#endif
								compile_score_csv = 0;	
							}

						}
						else {
							compile_score_csv = 0;
						}
#ifdef DEBUG_THREAD
						printf("점수는 : %lf\n", compile_score_csv);
#endif
					}

				}

				// 이제 점수를 점수 테이블에 저장한다.	
				fprintf(fp_result_table, ",");
				fprintf(fp_result_table, "%lf", compile_score_csv);
				sum_result += compile_score_csv;
				continue;
			}


			//////////////////////////////////////////////////////////////////////////////////////////////////
			//                    여기서부터는 빈칸문제를 채점하는 곳                                        /
			//////////////////////////////////////////////////////////////////////////////////////////////////


			// 문제의 정답 파일을 토큰 분리한다.
			// 문제의 정답 파일은 char *trueset_answer에 있음

			char *token_q;
			char *ptr_q[2];
			char tmp_q[100][100];
			char tmp2_q[100][100];
			token_q = strtok_r(trueset_answer, ":", &ptr_q[0]);
#ifdef DEBUG
			printf("trueset_answer = %s\n", trueset_answer);
#endif
			int i_q = 0;
			int j_q = 0;
			while (token_q) {
				strcpy(tmp_q[i_q], token_q);
				strcpy(tmp2_q[j_q], token_q);
#ifdef DEBUG
				printf("%d %s\n", j_q, tmp2_q[j_q]);
#endif
				token_q = strtok_r(tmp_q[i_q], " ", &ptr_q[1]);
				while (token_q) {
					strcpy(tmp_q[i_q], token_q);
					token_q = strtok_r(NULL, " ", &ptr_q[1]);
					i_q++;
				}
				token_q = strtok_r(NULL, ":", &ptr_q[0]);
				i_q++;
				j_q++;
			}
			//			strcpy(tmp_q[i_q], "THEENDISTHISPOINT!!!!");

			int space_problem_token = i_q;
			int problem_token = j_q;
#ifdef DEBUG
			printf("문제토큰 i는 %d, j는 %d\n", i_q, j_q);
#endif
			int tmp_size = strlen(tmp2_q[0]);
			if ( problem_token != 1 && problem_token > 0 ) {
				tmp2_q[0][tmp_size-1] = '\0';
			}

			int k_q = 0;
			for(i_q=1; i_q<problem_token; i_q++) {
				tmp_size = strlen(tmp2_q[i_q]);
				for(k_q=0; k_q<tmp_size-1; k_q++) {
					tmp2_q[i_q][k_q] = tmp2_q[i_q][k_q+1];
				}
				if (i_q == problem_token - 1) {
					tmp2_q[i_q][tmp_size-1] = '\0';
				}
				else {
					tmp2_q[i_q][tmp_size-2] = '\0';
				}
			}

#ifdef DEBUG
			for(i_q=0; i_q<10; i_q++)
				printf("tmp_q[%d]: %s\n", i_q, tmp_q[i_q]);

			for(i_q=0; i_q<10; i_q++)
				printf("tmp2_q[%d]: %s\n", i_q, tmp2_q[i_q]);
#endif
			int sw_answer = 0;
			for(i_q=0; i_q<problem_token; i_q++) {
				if(strcmp(tmp2_q[i_q], student_answer) == 0) {
					// 정답일 경우 score.csv 파일에 배점 입력
					fprintf(fp_result_table, ",%lf", score_table[i-2].score);
					sum_result += score_table[i-2].score;
#ifdef DEBUG_S
				//	printf("정답!\n");
#endif
					sw_answer = 1;
				}
				//#ifdef DEBUG
				//printf("%s---%s---\n", tmp2_q[i_q], student_answer);
				//#endif
			}

			//printf("파싱 전 sw_answer = %d\n", sw_answer);

			if (sw_answer == 0) {

				//printf("sw_answer가 0이네요! 파싱을 시작합시다!\n");
				//sleep(5);
				sw_answer = parsing(student_answer_path,answer_path);
				//printf("\n\n파싱한 결과는 %d입니다!(1 : 성공, 0 : 실패)\n\n", sw_answer);
				if (sw_answer != 0) {
					// 정답!
					fprintf(fp_result_table, ",%lf", score_table[i-2].score);
					sum_result += score_table[i-2].score;
					//printf("정답!\n");
				}
			}
			//printf("파싱 후 sw_answer = %d\n", sw_answer);

			if(sw_answer == 0) {
				// 오답일 경우
				//printf("오답!\n");
				fprintf(fp_result_table, ", 0");
				sum_result += 0;
#ifdef DEBUG_S
				//printf("오답!\n");
#endif
			}

			//		exit(0);
			fclose(fp_answer);
			fclose(fp_student);

			// 초기화
			for(i_q = 0; i_q < problem_token; i_q++) {
				strcpy(tmp2_q[i_q], "");
			}

			for(i_q = 0; i_q < space_problem_token; i_q++) {
				strcpy(tmp_q[i_q], "");
			}

THISEND:
			continue;
		}

		// score.csv 파일에 총점을 출력한다.
		fprintf(fp_result_table, ",%lf\n", sum_result);

		// 20190001 is finished... 가 출력되게끔 한다.
		printf("%s is finished...", studentlist[k]->d_name);

		// 만약 옵션 인자 중 -p옵션이 있을 경우, 각 학생의 점수를 출력한다.
		// 그리고 그 점수들을 p_total_score_for_student에 누적시킨다.
		// 평균은 모든 학생의 채점이 끝난 후 출력한다.
		if(opt_p == 1) {
			printf(" score : %lf", sum_result);
			p_total_score_for_student += sum_result;
		}


		// 만약 옵션 인자 중 -c옵션이 있을 경우, 해당하는 학생의 점수를 기억해 둔다.
		// 학번 : studentlist[k]->d_name
		// -c옵션 가변인자 개수 : j_2
		// -c옵션 가변인자를 저장하는 char[][] 배열 : _c

		// 해당 학생의 점수를 저장해 두는 double형 배열
		// -1.11로 초기화한다. 이는 절대 나올 수 없는 점수이므로, 만약 이 점수가 유지된다면
		// 그 학생이 점수 테이블에 없거나, 혹은 -c옵션 가변인자의 개수가 더 적은 것이다.
		// 이는 앞에서 초기화한 바 있었다.
		// double opt_c_score[5] = {-1.11, -1.11, -1.11, -1.11, -1.11};

		int opt_c_i;

		if (opt_c == 1) {
			for(opt_c_i = 0; opt_c_i < j_2; opt_c_i++) {
				// 만약 opt_c_score에 기록되지 않았다면, 그 학번인 사람은 없는 것이다.
				if(strcmp(_c[opt_c_i], studentlist[k]->d_name) == 0) {
					opt_c_score[opt_c_i] = sum_result;
				}
			}
		}


		printf("\n");
		answer_compile_flag = 1; // 정답 파일은 이제 더 이상 컴파일/실행되지 않아도 된다.
		// 디버깅용
		//		if ( k == 2 ) exit(0);
		//		exit(0);
	}

	// 정답을 읽는다.





	// -------------------------------------------------------------------------------//
	//                              프로그램 마무리 단계
	// -------------------------------------------------------------------------------//

	// -p옵션 마무리 - 전체 평균을 출력하기
	if (opt_p == 1) {
		printf("Total average : %lf\n", p_total_score_for_student / (student_count-2));
	}

	// -c옵션 마무리 - 해당하는 학생의 score를 출력하기
	if (opt_c == 1) {
		for (idx = 0; idx < j_2; idx++) {
			if(opt_c_score[idx] != -1.11)
				printf("%s's score : %lf\n", _c[idx], opt_c_score[idx]);
			else
				printf("%s's score doesn't exist!\n", _c[idx]);
		}
	}
	// 할당 취소

	fclose(fp_result_table);

	for (idx = 0; idx < student_count; idx++) {
		free(studentlist[idx]);
	}
	free(studentlist);
	closedir(dir_stu);

	for (idx = 0; idx < count; idx++) {
		free(namelist[idx]);
	}
	free(namelist);
	closedir(dir_ptr);

	// 프로그램 수행 시간 측정 끝, 표시
	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);

	// 프로그램 종료
	exit(0);
}
