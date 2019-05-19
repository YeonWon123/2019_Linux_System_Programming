#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "ssu_runtime.h"

#define DEBUG



//*************************************************************************************//
//                                  전역변수 선언                                      //
//*************************************************************************************//

// 기본 자료형을 저장해 두는 배열
#define SIZE 8	// 배열의 크기
char type[SIZE][30] = {"int[]", "int", "float", "double", "char[]", "char", "File ", "FileWriter "};

// 기본 자료형에 따라 형태를 저장해 두는 배열
char type_d[SIZE][20] = {"", "\"%d\"", "\"%f\"", "\"%lf\"", "\"%s\"", "\"%c\"", "FILE *", ""};

// 소스파일에 쓰인 변수와 자료형을 보관하는 구조체
typedef struct VAR_STORE {
	char types[20];
	char var[100];
} var_store;

// main함수, 인자로 filename과 option을 받는다.
int main(int argc, char *argv[]) {

	//***********************************************************************************//
	//                           main 함수 안에서의 지역변수 선언                        //
	//***********************************************************************************//

	// 옵션인자 플래그 지정
	int j_flag = 0;
	int c_flag = 0;
	int p_flag = 0;
	int f_flag = 0;
	int l_flag = 0;
	int r_flag = 0;

	// 각 옵션인자에 필요한 배열과 정수형 변수 지정
	char p_buffer[1024][1024];
	int p_count = 0;

	// 변환하고자 하는 java 소스 파일 디스크립터
	int fd_file;

	// 변환한 파일명 저장 - main함수 전용
	char fd_file_tf[1024];

	// 변환한 파일명 저장 - class함수 전용
	char fd_file_tf_c[1024];
	fd_file_tf_c[0] = '\0'; // '\0'으로 초기화

	// Makefile 파일명 저장
	char makefile_name[1024];	// main  (ex: q1,q2,q3)
	char makefile_name_c[1024];	// class (ex: Stack)

	// 반복문 등에 사용되는 변수 i, j, k, m, 임시적으로 쓰이는 int형 변수 temp, char형 배열 buf_temp
	int i, j, k, m, temp;
	char buf_temp[1024];

	// 버퍼, 버퍼 사이즈, 토큰을 저장하는 배열, 토큰의 개수와 줄 개수 그리고 fgetc 함수의 반환값을 받는 정수
	char buffer[1024][1024];
	int buffer_size[1024];
	char token[1024][1024];
	int token_count = 0;
	int line_count = 0;
	int ch_num = 97; // 아스키 코드로 97은 a를 의미한다.

	// 변환할 코드의 변수의 type과 이름을 저장하는 배열
	char type_buf[1024][500];

	// 변환된 코드를 저장하는 배열과 그 줄 개수를 나타내는 정수 - main함수 전용
	char transfer_buf[1024][1024];
	int transfer_line = 0;

	// 변환된 코드를 저장하는 배열과 그 줄 개수를 나타내는 정수 - class함수 전용
	char transfer_buf_c[1024][1024];
	int transfer_line_c = 0;

	// 변환된 코드에서 사용되는 헤더를 저장하는 배열과 그 개수를 나타내는 정수
	char header_store[1024][1024];
	int header_count = 0;

	// 변환된 코드에서 사용되는 헤더를 저장하는 배열과 그 개수를 나타내는 정수 - class함수 전용
	char header_store_c[1024][1024];
	int header_count_c = 0;

	// 변환된 코드에서 사용되는 헤더를 저장하는 배열과 그 개수를 나타내는 정수 - main함수 전용
	char header_store_main[1024][1024];
	int header_count_main = 0;

	// 코드에서 사용되는 변수의 이름과 자료형을 보관하는 구조체 배열과 그 개수 - main함수 전용
	var_store variables[1024];
	int variables_count = 0;

	// 코드에서 사용되는 변수의 이름과 자료형을 보관하는 구조체 배열과 그 개수 - class함수 전용
	var_store variables_c[1024];
	int variables_count_c = 0;

	// main함수가 등장했는지 여부를 판별하는 변수 지정 (등장하면 1)
	int main_sw = 0;

	// class함수가 등장했는지 여부를 판별하는 변수 지정 (등장하면 1)
	int class_sw = 0;

	// main 부분 줄 개수
	int main_count = 0;

	// class 부분 줄 개수
	int class_count = 0;
	int class_count_temp = 0; // 이 뒤로는 main함수 부분을 나타냄

	// 코드에서 등장한 함수를 저장하는 배열과 그 개수를 나타내는 정수 (최대 100개까지 저장가능)
	char func_store[100][1024];
	int func_count = 0;

	// 예외 처리(IOException)를 하는지의 여부를 판단하는 스위치 변수
	int exception_sw = 0; // 1이면 예외처리를 수행

	// 프로그램의 실행 시간을 측정하고자 하는 변수
	struct timeval begin_t, end_t;
	gettimeofday(&begin_t, NULL);

	//************************************************************************************//
	//                              옵션 인자를 받는 부분                                 //
	//************************************************************************************//
	
	// 인자의 개수가 2개 미만이면 예외처리
	if (argc < 2) {
		fprintf(stderr, "usage : %s <FILENAME> [OPTION]\n", argv[0]);
		exit(1);	
	}
	else {
		// 인자의 개수가 2개 이상일때, 존재하지 않는 파일을 대상으로 할 경우 예외처리
		if ((fd_file = open(argv[1], O_RDONLY, 0644)) < 0) {
			fprintf(stderr, "%s doesn't exist!\n", argv[1]);
			exit(1);
		}
		close(fd_file);

		// 인자의 개수가 3개 이상일때, 옵션 여부를 검토
		for (i = 2; i < argc; i++) {
			
			// -j 옵션일 경우
			if (strcmp(argv[i], "-j") == 0) {
				j_flag = 1;
			}
			// -c 옵션일 경우
			else if (strcmp(argv[i], "-c") == 0) {
				c_flag = 1;
			}
			// -p 옵션일 경우
			else if (strcmp(argv[i], "-p") == 0) {
				p_flag = 1;
			}
			// -f 옵션일 경우
			else if (strcmp(argv[i], "-f") == 0) {
				f_flag = 1;
			}
			// -l 옵션일 경우
			else if (strcmp(argv[i], "-l") == 0) {
				l_flag = 1;
			}
			// -r 옵션일 경우
			else if (strcmp(argv[i], "-r") == 0) {
				r_flag = 1;
			}
			else {
				fprintf(stderr, "%s isn't option!\n", argv[i]);
			}
		}	
	}

	//************************************************************************************//
	//                      java 소스파일을 c 소스파일로 변환하는 부분                    //
	//************************************************************************************//

	// 파일의 내용을 한 글자씩 읽어서 buf에 저장하기
	
	FILE* fd_input = fopen(argv[1], "r");
/*	i = 0;
	while(i < 100) {
		ch_num = fgetc(fd_input);
		buffer[0][i] = ch_num;
		i++;
	}

	printf("buffer: %s\n", buffer[0]);
	exit(0);
*/
	for (i = 0; ch_num != EOF; i++) {
		ch_num = ' ';
		for (j = 0; ch_num != '\n' && ch_num != EOF; j++) {
			ch_num = fgetc(fd_input);
#ifdef DEBUG
//			printf("%c", ch_num);
#endif
			buffer[i][j] = ch_num;
		}
		buffer[i][j] = '\0';
		buffer_size[i] = j+1;
		line_count++;
	}
#ifdef DEBUG
	printf("최종 라인 수 : %d\n", line_count-1);
#endif
	line_count = i-1;
/*
	// class 개수 파악하기
	// 만약 buffer[i]에 class가 있을 경우, class_count++
	// 그러나 System.out.printf(와 같이 있을 경우, class_count--;
	for (i = 0; i < line_count-1; i++) {
		if (strstr(buffer[i], "class ") != 0) {
			if (strstr(buffer[i], "System.out.printf(") == 0) {
				class_count++;
			}
		}
	}
#ifdef DEBUG
	printf("class_count(생성해야 하는 파일 수) : %d\n", class_count);
//	exit(0);
#endif
*/

#ifdef DEBUG
	// buf에 저장된 내용 읽어보기
	// 옵션구현(j옵션)
	for (i = 0; i < line_count; i++) {
		printf("%d %s", i+1, buffer[i]); // %s에 이미 \n이 있음
	}
//	exit(0);
#endif
	// 줄을 셀 때는 0이 아닌 1부터 시작하기 때문
	// 이 줄 개수는 -l 옵션에서 사용 가능

	// Anyway, 그 줄을 다음과 같이 분석한다.

	// 단, 앞에 나오는 괄호나 공백은 생략, 첫번째 문자가 나오는 순간부터 입력받기 시작할 것.
		// -> 일단 나중에, 직관적인 것부터 함 
	
	// main부분 줄, class부분 줄 변수 초기화 후 for문 진입
	main_count = 0;
	class_count = 0;	

	for (i = 0; i < line_count-1; i++) {

		// 먼저 분석하고자 하는 줄이 class 부분인지 main 부분인지를 확인
		// 즉, stack.c인지(class 코드인지) q2.c인지(main 코드인지) 확인
		if (main_sw == 1) {
			printf("main_count증가 : %d\n", main_count);
			main_count++;
		}	
		else if (class_sw == 1) {
			class_count++;
		}

		// 2019-04-08 완성	
		// (1)  printf가 있는지 살피기(System.out.printf가 존재하는가?) -> 있다면 (-p 옵션 보고 넣음) 변환
		
		strcpy(buf_temp, "System.out.printf(");
		temp = strlen(buf_temp);
		if (strstr(buffer[i], buf_temp) != NULL) {
#ifdef DEBUG
			printf("System.out.printf() 발견!\n");
#endif	
			strcpy(buf_temp, buffer[i]);
			// 원본에서, System.out.만 지우면 된다...
			// 앞에서부터 토큰분리를  .으로 2번 하자
			char *p_buf_temp;
			//char n_buf_temp[1024];
			p_buf_temp = strtok(buf_temp, "."); // system
			p_buf_temp = strtok(NULL, "."); // out
			p_buf_temp = strtok(NULL, "\n"); // printf(...

			// 만약 st.peek()처럼 인자로 st.과 같이 객체 함수가 들어올 경우
			// 앞에 st. 부분은 제거해 주어야 한다.
			// \"로 토큰분리하고, ,와 .로 토큰분리하여 없애주자
			char o_func[1024];
			
			// 만약 함수가 인자에 있다면 객체부분만 제거
			for (k = 0; k < func_count; k++) {
				if (strstr(p_buf_temp, func_store[k]) != 0) {
#ifdef DEBUG
					printf("인자로 함수가 있음!\n");
					printf("발견된 함수 : %s\n", func_store[k]);
#endif
					p_buf_temp = strtok(p_buf_temp, ",");
					strcpy(o_func, p_buf_temp);
					strcat(o_func, ", ");
					p_buf_temp = strtok(NULL, ".");
					p_buf_temp = strtok(NULL, "\n");
					strcat(o_func, p_buf_temp);
					strcat(o_func, "\n");
					p_buf_temp = strtok(o_func, "\n");
					break;
				}
			}

			// 만약 tab키가 있었다면 tab키 보정을 해 줘야 한다.
			// tab키가 몇번 눌렸는지 확인한 뒤, 그 수에서 1을 빼 준 수만큼 tab키를 할당한다.
			// 왜냐하면 자바는 class 선언부 덕분에 tab키가 한번 더 눌렸기 때문이다.
			int j = 0;
			if (buffer[i][0] == '\t') {
				j = 1;
				while(buffer[i][j] == '\t') {
#ifdef DEBUG
					printf("buffer[%d][%d] = %c\n", i, j, buffer[i][j]);
#endif
					j++;
				}
			}

			if (j > 1) {
				strcpy(transfer_buf[transfer_line], "\t");
			
				for (k = 2; k < j; k++) {
					strcat(transfer_buf[transfer_line], "\t");
				}

				strcat(transfer_buf[transfer_line], p_buf_temp);
			}
			else {
				strcpy(transfer_buf[transfer_line], p_buf_temp);
			}
	

			strcat(transfer_buf[transfer_line], "\n");
			transfer_line++;
		
			if (p_flag == 1) {
				strcpy(p_buffer[p_count], "System.out.printf() -> printf()");
				// 단, 이전에 나온 함수는 다시 쓰지 않음
				for (int kk = 0; kk < p_count; kk++) {
					if (strcmp(p_buffer[kk], p_buffer[p_count]) == 0) {
#ifdef DEBUG
						printf("같음!\n");
#endif
						p_count--;
						break;
					}
				}
#ifdef DEBUG
				for (int kk = 0; kk <= p_count; kk++) {
					printf("p_buffer[%d] = %s\n", kk, p_buffer[kk]);
				}
#endif
				p_count++;
				
			}
			continue;
		}

		// (1) scanf가 있는지 살피기(.nextInt()가 존재하는가?) -> 있다면 (-p 옵션 보고 넣음) 변환
		// 단 System.out.printf( 가 없어야 함 -> System.out.printf(의 우선 순위를 더 높게 함으로써 해결함
		
		strcpy(buf_temp, ".nextInt()");
		temp = strlen(buf_temp);
		if (strstr(buffer[i], buf_temp) != NULL) {
#ifdef DEBUG
			printf("변수명.nextInt() 발견!\n");
#endif
			// 탭키간격 맞추기
			j = 0;
			if (buffer[i][0] == '\t') {
				j = 1;
				while(buffer[i][j] == '\t') {
					j++;
				}
			}

			if (j > 1) {
				strcpy(transfer_buf[transfer_line], "\t");
				for(k = 2; k < j; k++) {
					strcat(transfer_buf[transfer_line], "\t");
				}

				strcat(transfer_buf[transfer_line], "scanf(");
			}
			else {
				strcpy(transfer_buf[transfer_line], "scanf(");
			}

			// scanf("%d", &num); 과 같은 형태이므로, 변수의 type과 이름을 알아야 한다.
			// 변수의 이름은 앞에서 알 수 있지만, 변수의 type은 앞에서 알아와야 한다.
			for (j = 0; j < variables_count; j++) {
#ifdef DEBUG
				printf("variables[j].var: %s, buffer[i]: %s\n", variables[j].var, buffer[i]);
#endif
				if(strstr(buffer[i], variables[j].var) != NULL) {
#ifdef DEBUG
					printf("변수를 %d번째에서 찾았다!\n", j);
#endif
					break;
				}
			}

			// var_store 구조체 배열에서 j번째에 있는 변수가 코드에 존재한다.
			strcat(transfer_buf[transfer_line], variables[j].types);	// "%d"
			strcat(transfer_buf[transfer_line], ", &");	// 쌍따옴표를 찍음
			strcat(transfer_buf[transfer_line], variables[j].var); // num
			strcat(transfer_buf[transfer_line], ");\n");

#ifdef DEBUG

			printf("\n\nDEBUG : variables[%d].var = %s\n", j, variables[j].var);
			printf("\n\nDEBUG : %s\n", transfer_buf[transfer_line]);
	//		for (j = 0; j < variables_count; j++) {
	//			printf("%s %s\n", variables[j].types, variables[j].var);
	//		}
#endif
			transfer_line++;
		
			if (p_flag == 1) {
				// 변수명 부분 고쳐야함! 변수를 아마 따로 저장하는 배열이 필요
				strcpy(p_buffer[p_count], variables[j].var);
				strcat(p_buffer[p_count], ".nextInt() -> scanf()");
#ifdef DEBUG
				for (int kk = 0; kk <= p_count; kk++)
					printf("p_buffer[%d] = %s\n", kk, p_buffer[kk]);
#endif
				// 단, 이전에 나온 함수는 다시 쓰지 않음
				for (int kk = 0; kk < p_count; kk++) {
					if(strcmp(p_buffer[kk], p_buffer[p_count]) == 0) {
						p_count--;
						break;
					}
				}
				p_count++;
				
			}
			continue;
		}

		// 2019-04-12 완성
		// (1) public class가 있는지 살피기 (있으면 public class 다음에 나오는 변수명.c 파일을 생성) - file_flag = 1; 로 만든다.
		strcpy(buf_temp, "public class");
		if (strstr(buffer[i], buf_temp) != NULL) {
#ifdef DEBUG
			printf("public class 발견!\n");
#endif
			main_sw = 1;
			class_sw = 0;
			// 앞에서 한 거 (printf)를 토대로, class 뒤의 이름을 보관할 변수를 만들고 할당
			// buffer[i]는 건드리면 안됨!
			strcpy(buf_temp, buffer[i]);
			
			// buf_temp에서 public class 다음부분을 추출하는데 {나 '\n'이 나오거나 '\0'이 나오면 break한다.
			char *p_buf_temp;
			char n_buf_temp[1024];
			p_buf_temp = strtok(buf_temp, " ");

			while(strcmp(p_buf_temp, "public") != 0) 
				p_buf_temp = strtok(NULL, " ");

			p_buf_temp = strtok(NULL, " ");	// "class" 부분 추출
			p_buf_temp = strtok(NULL, " ");	// "class" 다음 부분 추출
			strcpy(n_buf_temp, p_buf_temp);
			for (j = 0; n_buf_temp[j] != '{' && n_buf_temp[j] != '\n' && n_buf_temp[j] != '\0' && n_buf_temp[j] != ' '; j++);
			n_buf_temp[j] = '\0';

			// makefile_name에 일단 할당(q1, q2, q3만 추출했으니)
			strcpy(makefile_name, n_buf_temp);

			// (n_buf_temp 안의 값).c 파일을 생성하면 된다.
			strcat(n_buf_temp, ".c");

			// 그 변수명.c 파일을 만들자
			creat(n_buf_temp, 0666);
#ifdef DEBUG
			printf("%s 파일 생성!\n", n_buf_temp);
#endif
			strcpy(fd_file_tf, n_buf_temp);
			continue;
		}

		// (2) class가 있는지 살피기 (있으면 class 다음에 나오는 변수명.c 파일을 생성) - class_sw = 1;로 만든다.
		strcpy(buf_temp, "class ");
		if (strstr(buffer[i], buf_temp) != NULL) {
#ifdef DEBUG
			printf("class 발견!\n");
#endif
			// class 뒤의 이름을 보관할 변수를 만들고 할당
			strcpy(buf_temp, buffer[i]);

			// buf_temp에서 class 다음부분을 추출
			char *p_buf_temp;
			char n_buf_temp[1024];
			p_buf_temp = strtok(buf_temp, " {\n\0");
			p_buf_temp = strtok(NULL, " {\n\0");
#ifdef DEBUG
			printf("p_buf_temp : %s---\n", p_buf_temp);
//			exit(0);
#endif
			// 그 변수명.c 파일을 만들기
			strcpy(n_buf_temp, p_buf_temp);

			// makefile을 위해서 미리 저장해놓음(Stack.c에서 .c를 제외한 부분만 따로 저장)
			strcpy(makefile_name_c, p_buf_temp);

			// .c 할당
			strcat(n_buf_temp, ".c");	
			creat(n_buf_temp, 0666);
#ifdef DEBUG
			printf("%s 파일 생성!\n", n_buf_temp);
#endif
			strcpy(fd_file_tf_c, n_buf_temp);
			class_sw = 1;
			main_sw = 0;		
			continue;
		}

		// (2) public static final이 있는지 살피기 (있으면 전처리기로, #define 변수명 값 이렇게 선언한다.)
		// public static final int STACK_SIZE = 10;
		// -> #define STACK_SIZE 10
		strcpy(buf_temp, "public static final");
		if (strstr(buffer[i], buf_temp) != NULL) {
#ifdef DEBUG
			printf("public static final 발견!\n");
#endif
			strcpy(buf_temp, buffer[i]);

			// buf_temp에서 변수명과 할당값 추출
			char *p_buf_temp;
			char n_buf_temp[2][1024]; // n_buf_temp[0] : 변수명, n_buf_temp[1] : 할당값
			p_buf_temp = strtok(buf_temp, " ");	// public
			p_buf_temp = strtok(NULL, " ");		// static
			p_buf_temp = strtok(NULL, " ");		// final
			p_buf_temp = strtok(NULL, " ");		// 자료형(int)
			p_buf_temp = strtok(NULL, " "); 	// 변수명(STACK_SIZE)
			// 자료형 저장
			strcpy(n_buf_temp[0], p_buf_temp);
			p_buf_temp = strtok(NULL, " =;\0\n");	// 할당값(10)
			strcpy(n_buf_temp[1], p_buf_temp);

			strcpy(transfer_buf[transfer_line], "#define ");
			strcat(transfer_buf[transfer_line], n_buf_temp[0]);
			strcat(transfer_buf[transfer_line], " ");
			strcat(transfer_buf[transfer_line], n_buf_temp[1]);
			strcat(transfer_buf[transfer_line], "\n");
			
#ifdef DEBUG
			printf("%s\n", transfer_buf[transfer_line]);
//			exit(0);
#endif
			transfer_line++;
			continue;
		}

		// (1) public static void main이 있는지 살피기 (있으면 int main(int argc, char *argv[])를 생성) - main_flag = 1;로 만든다.
		strcpy(buf_temp, "public static void main");
		if (strstr(buffer[i], buf_temp) != NULL) {
#ifdef DEBUG
			printf("public static void main 발견!\n");
#endif
			strcpy(transfer_buf[transfer_line], "int main(int argc, char *argv[])");
			if (strstr(buffer[i], "{") != 0) {
				strcat(transfer_buf[transfer_line], " {\n");
			}
			else {
				strcat(transfer_buf[transfer_line], "\n");
			}
			transfer_line++;
			main_sw = 1;
			class_sw = 0;	

			if (strstr(buffer[i], "IOException") != 0) {
				exception_sw = 1;
			}

			continue;
		}

		// (2) public이 등장할 경우 - 이제 남은 경우는 함수인 경우밖에 없음
		// 함수는 따로 보관하고, 만약 st.push()같은 형이 등장할 경우 st는 제외하고 push만 넣으면 된다.
		// 그리고 main함수에서, 함수가 등장할 경우 그 함수에 맞게 써 주면 된다.
		strcpy(buf_temp, "public ");
		if (strstr(buffer[i], buf_temp) != NULL) {
#ifdef DEBUG
			printf("public 함수 발견!\n");
#endif
	
			strcpy(buf_temp, buffer[i]);
			// 토큰 분리
			char *p_buf_temp;
			char n_buf_temp[1024];
			p_buf_temp = strtok(buf_temp, " "); // public
			p_buf_temp = strtok(NULL, "\0");
#ifdef DEBUG
			printf("%s\n", p_buf_temp);
//			exit(0);
#endif
			// 만약 앞에 기본 자료형이 제시되지 않았다면 (생성자라면)
			// void를 써 준다.	
			// 단 File, FileWriter는 기본 자료형이 아니니 예외로 한다.
			int basic_type_sw = 0;
			for (k = 0; k < SIZE-2; k++) {
				if (strstr(p_buf_temp, type[k]) != NULL) {
					basic_type_sw = 1;
					break;
				}
			}

			if (basic_type_sw == 0) {
				strcpy(transfer_buf[transfer_line], "void ");
				strcat(transfer_buf[transfer_line], p_buf_temp);
				// 함수를 토큰분리한 후 저장, 이 경우, Stack(){ 같은 부류에서 Stack만 저장하는 것, (으로 토큰분리
				// 생성자의 경우는 (를 추가함
				p_buf_temp = strtok(p_buf_temp, "( ");
				strcat(p_buf_temp, "(");
				strcpy(func_store[func_count], " "); // 공백을 추가함으로써, Stack()과 printStack()의 차이 방지
				strcat(func_store[func_count], p_buf_temp);
				func_count++;
			}
			else {
				strcpy(transfer_buf[transfer_line], p_buf_temp);
				// 함수를 토큰분리한 후 저장, 이 경우, int peek()와 같은 부류에서 peek만 저장하는 것, 공백과 (으로 두번 토큰분리
				p_buf_temp = strtok(p_buf_temp, " (");
				p_buf_temp = strtok(NULL, " (");
				strcpy(func_store[func_count], "."); // .을 추가함으로써, .printStack()으로 인식하게 하여 Stack()으로 인식하게 하는것 방지
				strcat(func_store[func_count], p_buf_temp);
				func_count++;
			}
			
			transfer_line++;
			continue;
		}

		// (2) 함수가 등장할 경우
		// push(15)나 pop(), Stack(), printStack()과 같은 부류가 등장할 경우
		// 단, Stack()과 printStack()같은 경우는, printStack()이 먼저 등장하던가 해야 하는데...
		//
		// 1. 생성자일 경우 (생성자는 애초에 특별취급해주자)
		// Stack st = new Stack(); -> Stack();
		// 이 경우는 Stack(); 으로 하면 되므로, .으로 나누기 불가능한 경우 " ="로 토큰분리해서 뽑아내면 된다.
		//
		// 2. 객체함수일 경우
		// st.push(5) -> push(5);
		// st.pop() -> pop();
		// 이 경우는 .를 중심으로 2번 토큰분리해서 뽑아내면 된다.
		int find_func = 0;
		for (k = 0; k < func_count; k++) {
			// 함수가 발견되면
			if (strstr(buffer[i], func_store[k]) != 0) {
#ifdef DEBUG
				printf("함수발견!\n");
				printf("buffer[%d] = %s\n", i, buffer[i]);
#endif
				// 만약 생성자라면
				if (strstr(func_store[k], "(") != 0) {
					// 우선 공백부터 처리
					strcpy(buf_temp, buffer[i]);

					if (buf_temp[0] = '\t') {
						k = 0;
						while (buf_temp[k] != '\0') {
							buf_temp[k] = buf_temp[k+1];
							k++;
						}
					}

					// 공백 삽입
					k = 0;
					while (buf_temp[k] == '\t') {
						transfer_buf[transfer_line][k] = '\t';
						k++;
					}

					// " ="으로 토큰분리해주고 그 값을 넣는다.
					char *p_buf_temp;
					p_buf_temp = strtok(buf_temp, " =");

					while (strcmp(p_buf_temp, "new") != 0) {
						p_buf_temp = strtok(NULL, " =");
					}
					p_buf_temp = strtok(NULL, " =;"); // Stack()
					strcat(transfer_buf[transfer_line], p_buf_temp);
					strcat(transfer_buf[transfer_line], ";\n");
#ifdef DEBUG
					printf("%s\n", transfer_buf[transfer_line]);
		//			exit(0);
#endif
					transfer_line++;
				}
				else {  // 객체함수라면
					// 우선 공백부터 처리
					strcpy(buf_temp, buffer[i]);

					if (buf_temp[0] = '\t') {
						k = 0;
						while (buf_temp[k] != '\0') {
							buf_temp[k] = buf_temp[k+1];
							k++;
						}
					}

					// 공백 삽입
					k = 0;
					while(buf_temp[k] == '\t') {
						transfer_buf[transfer_line][k] = '\t';
						k++;
					}

					// "."으로 토큰분리해주고 그 값을 넣는다.
					char *p_buf_temp;
					p_buf_temp = strtok(buf_temp, ".");	// st
					p_buf_temp = strtok(NULL, ".");		// push(5);\n
					strcat(transfer_buf[transfer_line], p_buf_temp);
#ifdef DEBUG
					printf("%s\n", transfer_buf[transfer_line]);
//					exit(0);
#endif
					transfer_line++;
				}

				find_func = 1;
				break;
			}
		}
		if (find_func == 1) continue;


		// (1) import 무시
		// 단 위의 조건들이 없어야 함
		strcpy(buf_temp, "import");
		if (strstr(buffer[i], buf_temp) != NULL) {
#ifdef DEBUG
			printf("import 발견\n");
#endif
			continue; // 무시하고 다음줄로 진행
		}

		// (1) Scanner %s = new Scanner(System.in) 무시
		// 단 위의 조건들이 없어야 함
		strcpy(buf_temp, "new Scanner(System.in)");
		if (strstr(buffer[i], buf_temp) != NULL) {
#ifdef DEBUG
			printf("new Scanner(System.in) 발견!\n");
#endif
			continue;
		}


		// (2) new int[]처럼 동적 할당이 등장한 경우
		// malloc로 바꾸어 주어야 함
		strcpy(buf_temp, "new ");
		if (strstr(buffer[i], buf_temp) != NULL) {
#ifdef DEBUG
			printf("new 발견!\n");
#endif

			// 우선 기존 자료형이 있는지 생각해보자. 만약 없다면, 무시한다.
			// 즉 Stack st = new Stack(); 이나 File file = new file("q3java.txt"); 같은 경우를 예외처리하는 것이다.
			// 이 경우들은 밑에 있는 continue를 따라가지 않고 그냥 쭉 밑으로 진행하면 된다. (if ~ else로 분기)
			int basic_type_sw = 0;
			for (k = 0; k < SIZE - 2; k++) {
				if (strstr(buffer[i], type[k]) != NULL) {
					basic_type_sw = 1;
					break;
				}
			}

			if (basic_type_sw == 1) {
				// stack = new int[STACK_SIZE];
				// -> stack = (int*)malloc(sizeof(int)*STACK_SIZE);
				// 공백도 생각해야 함, 일단, =까지 가자.
				strcpy(buf_temp, buffer[i]);
				if (buf_temp[0] == '\t') {
					k = 0;
					while (buf_temp[k] != '\0') {
						buf_temp[k] = buf_temp[k+1];
						k++;
					}
				}

				// 공백 삽입
				k = 0;
				while (buf_temp[k] == '\t') {
					transfer_buf[transfer_line][k] = '\t';
					k++;	
				}

				// 토큰분리를 new로 해서, 그 전꺼는 그냥 넣고, 그 다음 토큰분리를 [로 하면 int 같은 자료형을 추출할 수 있음
				// 그 다음 토큰분리를 ]로 하면 STACK_SIZE와 같은 변수명을 찾을 수 있음
				char *p_buf_temp;
				char n_buf_temp[1024];
				char n_type_temp[100];
				p_buf_temp = strtok(buf_temp, "\t ");
				if (strcmp(p_buf_temp, "new") != 0) {
					strcat(transfer_buf[transfer_line], p_buf_temp);
					strcat(transfer_buf[transfer_line], " ");
					p_buf_temp = strtok(NULL, "\t []");	
				}

				p_buf_temp = strtok(NULL, "\t []");	// new
				p_buf_temp = strtok(NULL, "\t []");	// int와 같은 자료형이 p_buf_temp에 저장됨
				strcpy(n_type_temp, p_buf_temp);	// 자료형은 따로 저장
				p_buf_temp = strtok(NULL, "\t []");	// STACK_SIZE와 같은 변수명이 p_buf_temp에 저장됨

				strcat(transfer_buf[transfer_line], "= (");
				strcat(transfer_buf[transfer_line], n_type_temp);
				strcat(transfer_buf[transfer_line], "*)malloc(sizeof(");
				strcat(transfer_buf[transfer_line], n_type_temp);
				strcat(transfer_buf[transfer_line], ")*");
				strcat(transfer_buf[transfer_line], p_buf_temp);
				strcat(transfer_buf[transfer_line], ");\n");
#ifdef DEBUG
				printf("%s\n", transfer_buf[transfer_line]);
				//			exit(0);
#endif
				transfer_line++;

				// p_flag가 1이라면
				// new %s[]-> malloc()
				if (p_flag == 1) {
					strcpy(p_buffer[p_count], "new ");
					strcat(p_buffer[p_count], n_type_temp);
					strcat(p_buffer[p_count], "[] -> malloc()");

					for (int kk = 0; kk < p_count; kk++) {
						if (strcmp(p_buffer[kk], p_buffer[p_count]) == 0) {
							p_count--;
							break;
						}
					}

					p_count++;
				}
				continue;
			}
		}

		// 바로 위의 경우(동적 할당을 위해 int, char, ...가 쓰이지 않은 경우)
		// (1) int, char, double, float, ... 등등 자료형과 변수명은 따로 저장해두기 (scanf시 사용해야 함)
		// 물론 그대로 출력하기
		int define_sw = 0;
		for (j = 0; j < SIZE; j++) {
			if (strstr(buffer[i], type[j]) != NULL) {
				define_sw = 1;
				char buffer_temp[1024];
#ifdef DEBUG
				printf("자료형 발견!\n");
				printf("발견된 자료형 : %s\n", type[j]);
#endif
				// 일단 출력
				strcpy(buffer_temp, buffer[i]);
				if (buffer[i][0] == '	') {
					k = 0;
					while (buffer_temp[k] != '\0') {
						buffer_temp[k] = buffer_temp[k+1];
						k++;
					}
				}

				// 만약 int[]형일 경우 int* 형으로 변경한다. (컴파일 에러 방지)
				if (j == 0) {
					printf("int[] 발견!\n");
					// 공백 삽입
					k = 0;
					while (buffer_temp[k] == '\t') {
						transfer_buf[transfer_line][k] = '\t';
						k++;
					}
					if (k != 0)
						strcat(transfer_buf[transfer_line], "int* ");
					else
						strcpy(transfer_buf[transfer_line], "int* ");

					// 그 다음꺼는 그냥 넣으면 됨.
					char *p_b_temp;
					p_b_temp = strtok(buffer_temp, "]");
					p_b_temp = strtok(NULL, " \n");
					strcat(transfer_buf[transfer_line], p_b_temp);
					strcat(transfer_buf[transfer_line], "\n");
					transfer_line++;
				}
				// 단, File은 char로 변경하자("File "을 확인하는게 좋을듯)
				// 즉, j == 6인 경우와 j == 7인 경우는 따로 처리하자.
				// j == 6인경우 : File, j == 7인경우 : FileWriter
				else if (j == 6) { // "File "
#ifdef DEBUG
					printf("File 발견!\n");
#endif
					// File file = new File("q3java.txt");
					// -> char *file = "q3java.txt"; 로 변경
					char buffer_temp2[1024];
					k = 0;
					// 공백 삽입
					while (buffer_temp[k] == '	') {
						transfer_buf[transfer_line][k] = '	';
						k++;	
					}

					// char *삽입
					if (k == 0)
						strcpy(transfer_buf[transfer_line], "char *");
					else
						strcat(transfer_buf[transfer_line], "char *");

					// 변수명, " = ", "파일명", ;, \n 삽입
					// 토큰분리를 해야 함
					char buffer_token[100][1024];
					char *p_buffer_token;
					int buffer_token_count = 0;
					strtok(buffer_temp, "\t =()");
					while ((p_buffer_token = strtok(NULL, "\t =()")) != NULL) {
						strcpy(buffer_token[buffer_token_count], p_buffer_token);
#ifdef DEBUG
						printf("%s---\n", buffer_token[buffer_token_count]);
#endif
						buffer_token_count++;
					}
#ifdef DEBUG
					for(k = 0; k < buffer_token_count; k++)
						printf("%s\n", buffer_token[k]);
#endif
					// 토큰분리되면, 변수명 new File "파일명" ;\n 이렇게 분리된다.
					// 이중 필요한 것은 변수명과 "파일명" 이다.
					strcat(transfer_buf[transfer_line], buffer_token[0]);
					strcat(transfer_buf[transfer_line], " = ");
					strcat(transfer_buf[transfer_line], buffer_token[3]);
					strcat(transfer_buf[transfer_line], ";\n");
					transfer_line++;						

				}
				else if (j == 7) { // "FileWriter "
					// FileWriter writer = new fileWriter(file, false);
					// -> FILE *writer = fopen(file, "w+");
					// FileWriter writer = new FileWriter(file, true);
					// -> FILE *writer = fopen(file, "r+");
					// 
					// 이후 만약 exception_sw = 1이라면
					// 예외처리하는 코드를 집어넣는다.
					// 단 탭 간격을 맞춰야 하므로,
					// 앞에서 탭 간격을 조사한 뒤 그 갯수를 저장했다가 그대로 가져온다.
					int tab_count = 0;

					// 우선 토큰분리를 시도하자
					char buffer_token[100][1024];
					char *p_buffer_token;
					int buffer_token_count = 0;
					strtok(buffer_temp, "\t =(),");
					while((p_buffer_token = strtok(NULL, "\t =(),")) != NULL) {
						strcpy(buffer_token[buffer_token_count], p_buffer_token);
#ifdef DEBUG
						printf("%s---\n", buffer_token[buffer_token_count]);
#endif
						buffer_token_count++;
					}
#ifdef DEBUG
					for (k = 0; k < buffer_token_count; k++)
						printf("%s\n", buffer_token[k]);
#endif
					// 토큰분리 결과
					// 변수명1 new FileWriter 변수명2 true(false) ;\n
					// 이때 필요한 것은 변수명1, 변수명2, true(false)
					
					// 먼저 공백을 삽입
					k = 0;
					while(buffer_temp[k] == '	') {
						transfer_buf[transfer_line][k] = '	';
						k++;
					}
					tab_count = k;
					// FILE *삽입
					strcat(transfer_buf[transfer_line], "FILE *");
					// 변수명1 삽입
					strcat(transfer_buf[transfer_line], buffer_token[0]);	// writer (변수명1)
					// = fopen( 삽입
					strcat(transfer_buf[transfer_line], " = fopen(");
					// 변수명2 삽입
					strcat(transfer_buf[transfer_line], buffer_token[3]);	// file (변수명2)
					// true이면 r+로, false이면 w+로
					if (strcmp(buffer_token[4], "true") == 0)
						strcat(transfer_buf[transfer_line], ", \"r+\");\n");
					else
						strcat(transfer_buf[transfer_line], ", \"w+\");\n");

					transfer_line++;

					// p_flag == 1일시
					// FileWriter() -> fopen()
					if (p_flag == 1) {
						strcpy(p_buffer[p_count], "FileWriter() -> fopen()");
						// 중복 여부 검사
						for (int kk = 0; kk < p_count; kk++) {
							p_count--;
							break;
						}	
						p_count++;
					}

					// 예외 처리 부분
					if (exception_sw == 1) {
						for (k = 0; k < tab_count; k++)
							transfer_buf[transfer_line][k] =  '	';
						
						strcat(transfer_buf[transfer_line], "if (access(");
						strcat(transfer_buf[transfer_line], buffer_token[3]); // file (변수명2)
						strcat(transfer_buf[transfer_line], ", F_OK) < 0) {\n");
						transfer_line++;
						
						for(k = 0; k <= tab_count; k++)
							transfer_buf[transfer_line][k] = '	';

						strcat(transfer_buf[transfer_line], "fprintf(stderr, \"error\\n\");\n");
						transfer_line++;
						

						for (k = 0; k <= tab_count; k++)
							transfer_buf[transfer_line][k] = '	';
						strcat(transfer_buf[transfer_line], "exit(1);\n");
						transfer_line++;

						for (k = 0; k < tab_count; k++)
							transfer_buf[transfer_line][k] = '	';
						strcat(transfer_buf[transfer_line], "}\n");
						transfer_line++;

						if (main_sw == 1) {
							main_count = main_count + 5;
						}
						else if (class_sw == 1) {
							class_count = class_count + 5;
						}

					}
				}
				else {
#ifdef DEBUG
					printf("일단 그대로 투입!\n");
#endif
					/*
					strcpy(buffer_temp, buffer[i]);
					if (buffer[i][0] == '	') {
						k = 0;
						while (buffer_temp[k] != '\0') {
							buffer_temp[k] = buffer_temp[k+1];
							k++;
						}
					}
					*/
					strcpy(transfer_buf[transfer_line], buffer_temp);
					transfer_line++;

					// 자료형과 변수명을 따로 저장하기

					strcpy(buf_temp, buffer_temp);

					// int num;		 -> type[j] : int, t_buf_temp : num
					// int even = 0, odd = 0;-> type[j] : int, t_buf_temp : even
					// 			 -> type[j] : int, t_buf_temp : odd
					//
					// ;로 끝내고, " ,=;" 등으로 토큰분리를 해야 할 듯
					char *p_buf_temp;
					char t_buf_temp[100][512]; // 토큰분리된 것을 먼저 저장
					p_buf_temp = strtok(buf_temp, " "); // int 와 num, even 등 구분하는 거니깐 space bar만 해도 됨

					int buf_counts = 0;
					while((p_buf_temp = strtok(NULL, " ")) != NULL) {
#ifdef DEBUG
						printf("%s\n", p_buf_temp);
#endif
						strcpy(t_buf_temp[buf_counts], p_buf_temp);
						buf_counts++;
					}

#ifdef DEBUG
					for (k = 0; k < buf_counts; k++) {
						printf("%s\n", t_buf_temp[k]);
					}
					// exit(0);
#endif
					// t_buf_temp에 저장되어 있는 값들을 토큰분리, " "나 "=" 앞에것만 토큰분리하면 된다(변수명만 추출)
					for (k = 0; k < buf_counts; k++) {
						strcpy(t_buf_temp[k], strtok(t_buf_temp[k], " =;"));
					}

#ifdef DEBUG
					for (k = 0; k < buf_counts; k++) {
						printf("%s\n", t_buf_temp[k]);
					}
					//exit(0);
#endif

					// 추출한 변수명 그리고 자료형(%d, %f, %lf 등등..) 저장
					for (k = 0; k < buf_counts; k++) {
						strcpy(variables[variables_count].types, type_d[j]);
						strcpy(variables[variables_count].var, t_buf_temp[k]);
						variables_count++;
					}
				}

				break;
			}
		}

		if(define_sw == 1) continue;	

		// (3) .write가 나오면 fprintf로 변경한다.
		// writer.write("2019 OSLAB\n"); -> fprintf(writer, "2019 OSLAB\n");
		// writer.write() -> fprintf()
		strcpy(buf_temp, ".write");
		if (strstr(buffer[i], buf_temp) != 0) {
#ifdef DEBUG
			printf(".write 발견!\n");
#endif
			strcpy(buf_temp, buffer[i]);
			k = 0;
			if (buf_temp[0] == '	') {
				k = 0;
				while(buf_temp[k] != '\0') {
					buf_temp[k] = buf_temp[k+1];
			//		printf("buf_temp[%d] = %c\n", k, buf_temp[k]);
					k++;
				}
			}

			// 공백 삽입
			k = 0;
			while (buf_temp[k] == '	') {
				transfer_buf[transfer_line][k] = '	';
				k++;
			}

			// fprintf( 삽입
			if (k == 0)
				strcpy(transfer_buf[transfer_line], "fprintf(");
			else
				strcat(transfer_buf[transfer_line], "fprintf(");

			// 토큰분리
			char buffer_token[100][1024];
			char *p_buffer_token;
			char p_name_token[100];	// p_flag == 1일때 p_buffer에 넣을 변수명
			int buffer_token_count = 0;
			p_buffer_token = strtok(buf_temp, "\t .()");
			strcpy(buffer_token[buffer_token_count], p_buffer_token);
			strcpy(p_name_token, p_buffer_token);

			buffer_token_count++;
			while((p_buffer_token = strtok(NULL, "\t .()")) != NULL) {
#ifdef DEBUG
				printf("%s\n", p_buffer_token);
#endif
				strcpy(buffer_token[buffer_token_count], p_buffer_token);	
				buffer_token_count++;
			}

#ifdef DEBUG
			for (k = 0; k < buffer_token_count; k++)
				printf("%s\n", buffer_token[k]);
		//	exit(0);
#endif
		//	분리된 토큰은 다음과 같다.
		//	변수명 write "2019 oslab\n" ;
		

			// 변수명 삽입
			strcat(transfer_buf[transfer_line], buffer_token[0]);
			strcat(transfer_buf[transfer_line], ", ");
			// 쓰여지는 부분 삽입, \"가 나오기 전까지
			for (k = 0; ; k++) {
				if (buffer_token[2+k][0] == ';') break;
				strcat(transfer_buf[transfer_line], buffer_token[2+k]);
				if (buffer_token[3+k][0] == ';') continue; // 마지막은 공백을 붙이지 않음
				strcat(transfer_buf[transfer_line], " ");
			}
			
			strcat(transfer_buf[transfer_line], ");\n");
			transfer_line++;

			// p_flag 처리
			if (p_flag == 1) {
				strcpy(p_buffer[p_count], p_name_token);
				strcat(p_buffer[p_count], ".write() -> fprintf()");
				for (k = 0; k < p_count; k++) {
					if (strcmp(p_buffer[p_count], p_buffer[k]) == 0) {
						p_count--;
						break;
					}
				}
				p_count++;
			}
			continue;
		}
		

		// (3) .flush가 나오면 fflush로 변경한다.
		// writer.flush() -> fflush(writer);
		// writer.flush() -> fflush()
		strcpy(buf_temp, ".flush");
		if (strstr(buffer[i], buf_temp) != 0) {
#ifdef DEBUG
			printf(".flush 발견!\n");
#endif
			strcpy(buf_temp, buffer[i]);
			k = 0;
			if (buf_temp[0] == '\t') {
				k = 0;
				while(buf_temp[k] != '\0') {
					buf_temp[k] = buf_temp[k+1];
					k++;
				}
			}

			// 공백(탭) 삽입
			k = 0;
			while (buf_temp[k] == '\t') {
				transfer_buf[transfer_line][k] = '\t';
				k++;
			}

			// fflush( 삽입
			if (k == 0)
				strcpy(transfer_buf[transfer_line], "fflush(");
			else
				strcat(transfer_buf[transfer_line], "fflush(");

			// 토큰분리 - 변수명 필요
			char buffer_token[1024];
			char *p_buffer_token;
			p_buffer_token = strtok(buf_temp, "\t .");
			strcpy(buffer_token, p_buffer_token);

			strcat(transfer_buf[transfer_line], buffer_token);
			strcat(transfer_buf[transfer_line], ");\n");
			transfer_line++;

			// p_flag 처리
			if (p_flag == 1) {
				strcpy(p_buffer[p_count], buffer_token);
				strcat(p_buffer[p_count], ".flush() -> fflush()");
				for (k = 0; k < p_count; k++) {
					if (strcmp(p_buffer[p_count], p_buffer[k]) == 0) {
						p_count--;
						break;
					}
				}
				p_count++;
			}
			continue;
		}		

		// (3) .close가 나오면 fclose로 변경한다.
		// writer.close() -> fclose(writer);
		// writer.close() -> fclose()
		strcpy(buf_temp, ".close");
		if (strstr(buffer[i], buf_temp) != 0) {
#ifdef DEBUG
			printf(".close 발견!\n");
#endif
			strcpy(buf_temp, buffer[i]);
			k = 0;
			if (buf_temp[0] == '\t') {
				k = 0;
				while (buf_temp[k] != '\0') {
					buf_temp[k] = buf_temp[k+1];
					k++;
				}
			}
			
			// 공백 삽입
			k = 0;
			while (buf_temp[k] == '\t') {
				transfer_buf[transfer_line][k] = '\t';
				k++;
			}

			// fclose( 삽입
			if (k == 0)
				strcpy(transfer_buf[transfer_line], "fclose(");
			else
				strcat(transfer_buf[transfer_line], "fclose(");

			// 토큰분리 - 변수명 필요
			char buffer_token[1024];
			char *p_buffer_token;
			p_buffer_token = strtok(buf_temp, "\t .");
			strcpy(buffer_token, p_buffer_token);

			strcat(transfer_buf[transfer_line], buffer_token);
			strcat(transfer_buf[transfer_line], ");\n");
			transfer_line++;

			// p_flag 처리
			if (p_flag == 1) {
				strcpy(p_buffer[p_count], buffer_token);
				strcat(p_buffer[p_count], ".close() -> fclose()");
				for (k = 0; k < p_count; k++) {
					if (strcmp(p_buffer[p_count], p_buffer[k]) == 0) {
						p_count--;
						break;
					}
				}
				p_count++;
			}
			continue;
		}



		// (1) return이 나오면 exit(0);으로 변경한다. 
		// 단, main함수가 등장한다는 조건이어야 한다.
		// 왜냐하면, (2)에서 전혀 다른 return이 등장하기 때문이다.
		// 따라서 그 경우는 냅두고, main함수에서 등장하는 return만 exit로 처리해준다.
		strcpy(buf_temp, "return");
		if (strstr(buffer[i], buf_temp) != 0 && main_sw == 1) {
#ifdef DEBUG
			printf("return 발견!\n");
#endif
			j = 0;
			if (buffer[i][0] == '\t') {
				j = 1;
				while(buffer[i][j] == '\t') j++;
			}

			if (j > 1) {
				strcpy(transfer_buf[transfer_line], "\t");
				for (k = 2; k < j; k++) {
					strcat(transfer_buf[transfer_line], "\t");
				}
				strcat(transfer_buf[transfer_line], "exit(0);\n");
			}
			else {
				strcpy(transfer_buf[transfer_line], "exit(0);\n");
			}
			transfer_line++;
			continue;
		}

		// (1) 나머지는 그대로 투입
#ifdef DEBUG
		printf("나머지는 그대로 투입!\n");
#endif
		if (main_sw == 0 && strstr(buffer[i], "{") != 0) continue;

		// 맨 마지막 tab키는 간격을 맞춰준다.
		if (i == line_count-2) {
			strcpy(transfer_buf[transfer_line], "}");
		}
		else {
			// 나머지도 /t 한칸씩을 당겨준다. 단 당길수 없으면 당기지 않는다.
			// java의 경우는 class 정의 안에 main함수가 있어서 c에서는 보정이 필요하다.
			char buffer_temp[1024];
			int tab_sw = 0;
			if (buffer[i][0] == '	') {
#ifdef DEBUG
				printf("탭키발견\n");
				printf("이전 buffer[%d] = %s\n", i, buffer[i]);
#endif
				tab_sw = 1;
				int j = 0;
				strcpy(buffer_temp, buffer[i]);
				while(buffer_temp[j] != '\0') {
					buffer_temp[j] = buffer_temp[j+1];
					j++;
				}
#ifdef DEBUG
				printf("바뀐 buffer_temp = %s\n", buffer_temp);
#endif
			}
			if (tab_sw == 1)
				strcpy(transfer_buf[transfer_line], buffer_temp);
			else
				strcpy(transfer_buf[transfer_line], buffer[i]);
		}

		transfer_line++;
	}

#ifdef DEBUG
	printf("\n\n 변환된 코드를 확인해보자!\n");
	for (k = 0; k < transfer_line; k++) {
		printf("%d %s", k, transfer_buf[k]);
	}

	printf("\nclass부분 줄 개수 : %d\n", class_count);
	printf("main부분 줄 개수 : %d\n", main_count);

	printf("\n\nclass부분 코드\n\n");
	for (k = 0; k < class_count; k++) {
		printf("%d %s", k+1, transfer_buf[k]);
	}

	printf("\n\nmain부분 코드\n\n");
	for (; k < main_count + class_count; k++) {
		printf("%d %s", k-class_count+1, transfer_buf[k]);
	}
//	exit(0);
#endif

	// 이제 파일에 한번 써보자
	// 1. q1, q3처럼 class가 없고 main부분만 있는 경우
	if (class_count == 0) {
		// 맨 첫 줄 : 헤더 테이블을 참조하며 프로그램에서 필요한 헤더를 자동으로 생성

		// transfer_buf에 있는 내용들을 0줄부터 transfer_line-1줄까지 생성한 파일에 쓴다.
		// 단, 이 설정은 (2)를 할 때는 바꿔야 한다. (왜냐하면, 먼저 stack.c 파일에 쓴 뒤 q2 파일에 쓰기 때문)
		FILE* fp_result = fopen(fd_file_tf, "r+");
		FILE* fp_headertable = fopen("headertable.txt", "r");
		char function_str[512];
		char *function_str_part;
		char str_save[512];

		// 헤더 파일을 생성 - 헤더 파일 찾기
		while(1) {
#ifdef DEBUG
			printf("while문 시작!\n");
#endif
			// 한 줄씩 읽음
			if (fgets(function_str, sizeof(function_str), fp_headertable) == NULL) {
#ifdef DEBUG
				printf("헤더 테이블을 다 읽음!\n");
#endif
				break;
			}
			// 원본 보관
			strcpy(str_save, function_str);
			function_str_part = strtok(function_str, " "); // open read write close exit printf scanf ...
			strcat(function_str_part, "(");	// printf나 scanf 안에 문자열로 들어간건 아니므로, (를 붙여서 함수인지 검증
			int done = 0; // 특정 함수를 찾았을 경우 1
			// 특정 함수가 있는지 확인, 있으면 그 함수에 맞는 헤더 파일을 넣음
			for (k = 0; k < transfer_line && done == 0; k++) {
#ifdef DEBUG
	//			printf("특정함수 확인 : 확인할 함수 : %s\n", function_str_part);
	//			printf("검사할 대상 : %s\n", transfer_buf[k]);
	//			printf("검사결과 : %s\n", strstr(transfer_buf[k], function_str_part));
	//			printf("검사결과 : %s\n", strstr(function_str_part, transfer_buf[k]));
#endif
				if (strstr(transfer_buf[k], function_str_part) != NULL) {
					// 만약, open, read, write, close, printf의 경우
					// fopen, fread, fwrite, fclose, fprintf에서 검출된 것인지 확인
					// 만약 그렇다면 continue로 무시
					// 다시 말해서, 함수가 fopen인데 open의 헤더 파일이 들어가는 것을 방지
					char f_str_part[1024] = "f";
					strcat(f_str_part, function_str_part);
					if (strstr(transfer_buf[k], f_str_part) != NULL) {
						continue;
					}

					// 옆에 있는 헤더를 넣어야 하는데, 이게 있는지를 살펴봄
#ifdef DEBUG
					printf("특정함수 발견! 발견된 함수는 %s\n", function_str_part);
#endif
					done = 1;
					int head_sw = 0;
					function_str_part = strtok(str_save, " ");
					function_str_part = strtok(NULL, "\n\0"); // #include <stdio.h> ...
					for (int n = 0;	n < header_count; n++) {
						if (strstr(header_store[n], function_str_part) != NULL) {
							head_sw = 1;
							break;
						}
					}
					// 없으면 추가
					if (head_sw == 0) {
						// 만약 헤더가 여러개일 경우, #가 있으면 \n으로 구분해 준다.
						function_str_part = strtok(function_str_part, "#");
						strcpy(header_store[header_count], "#");
						strcat(header_store[header_count], function_str_part);
						while((function_str_part = strtok(NULL, "#")) != NULL) {
							header_count++;
							strcpy(header_store[header_count], "#");
							strcat(header_store[header_count], function_str_part);
						}
#ifdef DEBUG
						printf("헤더 파일 추가!\n");
						printf("%d 번째 추가한 헤더 파일 : %s\n", header_count+1, header_store[header_count]);
						//	if (header_count > 0) printf("이전의 헤더 파일 : %s\n", header_store[header_count-1]);
#endif
						header_count++;
						break;
					}
				}

				if (done == 1) break;
			}
		}
		fclose(fp_headertable);

#ifdef DEBUG
		printf("생성해야 하는 헤더 파일\n");
		for (k = 0; k < header_count; k++) {
			printf("%d 번째 : %s\n",k+1, header_store[k]);
		}

#endif
		// 헤더 파일을 생성 - 파일에 쓰기
		for (k = 0; k < header_count; k++) {
			fprintf(fp_result, "%s\n", header_store[k]);
#ifdef DEBUG
			int new_line = 1;
			// -c 옵션
			printf("%d %s\n", new_line++, header_store[k]);	
#endif
		}

		// 본문을 쓰자
		for (k = 0; k < transfer_line; k++) {
			fprintf(fp_result, "%s", transfer_buf[k]);
		}

		fclose(fp_result);

		// 파일명_Makefile도 함께 생성하기(변환된 C 언어 파일을 컴파일할 수 있어야 함)
		// 실행은 make 파일명 (make q1) 으로 가능
		char m_original[1024];
		strcpy(m_original, makefile_name);

		strcat(makefile_name, "_Makefile");
		FILE *fd_makefile;
		char makefile_temp[1024];
		fd_makefile = fopen(makefile_name, "w+");
		fprintf(fd_makefile, "CC = gcc\n"); 
		fprintf(fd_makefile, "OBJECTS = %s.o\n", m_original);
		fprintf(fd_makefile, "TARGET = %s.out\n\n", m_original);
		fprintf(fd_makefile, ".SUFFIXES : .c .o\n\n");
		fprintf(fd_makefile, "all : $(TARGET)\n\n");
		fprintf(fd_makefile, "$(TARGET): $(OBJECTS)\n");
		fprintf(fd_makefile, "	$(CC) -o $@ $(OBJECTS)\n\n");
		fprintf(fd_makefile, "clean:\n");
		fprintf(fd_makefile, "	rm -f $(OBJECTS) $(TARGETS)");
		fclose(fd_makefile);	

		// 특정 옵션이 지정되어 있을 경우, 그 옵션에 맞는 행동을 할당할 것

		// 프로그램 마무리
#ifdef DEBUG
		printf("\nDEBUG종료\n");
		printf("---------------------------------------------------------\n");
#endif	
		printf("%s converting is finished!\n", fd_file_tf);
	}
	else {
		// 2. q2처럼 class부분과 main부분이 같이 있는 경우
		// 이 경우는 class부분함수와 main함수를 같이 써야 한다.
		// 먼저 class부분 파일을 열어서 써 보자!
		FILE *fp_result = fopen(fd_file_tf_c, "r+");
		FILE *fp_headertable = fopen("headertable.txt", "r");
		char function_str[512];
		char *function_str_part;
		char str_save[512];

		// class부분 헤더 파일 생성 - 헤더 파일 찾기
		while(1) {
			// 한 줄씩 읽는다.
			if (fgets(function_str, sizeof(function_str), fp_headertable) == NULL) {
				// 헤더 테이블을 다 읽은 경우
				break;
			}

			// 원본 보관
			strcpy(str_save, function_str);
			function_str_part = strtok(function_str, " ");
			strcat(function_str_part, "(");
			int done = 0;
			// 특정 함수가 있는지 확인한다. 먼저 class부분에서 확인해 보자.
			for (k = 0; k < class_count && done == 0; k++) {
				if (strstr(transfer_buf[k], function_str_part) != NULL) {
					// 만약, open, read, write, close, printf의 경우
					// fopen, fread, fwrite, fclose, fprintf에서 검출된 것인지 확인
					// 만약 그렇다면 continue로 무시
					// 다시 말해서, 함수가 fopen인데 open의 헤더 파일이 들어가는 것을 방지
					char f_str_part[1024] = "f";
					strcat(f_str_part, function_str_part);
					if (strstr(transfer_buf[k], f_str_part) != NULL) {
						continue;
					}


					// 옆에 있는 헤더를 넣어야 하는데, 이게 있는지를 살펴봄
#ifdef DEBUG
					printf("특정함수 발견! 발견된 함수는 %s\n", function_str_part);
#endif
					done = 1;
					int head_sw = 0;
					function_str_part = strtok(str_save, " ");
					function_str_part = strtok(NULL, "\n\0"); // #include <stdio.h> ...
					for (int n = 0;	n < header_count; n++) {
						if (strstr(header_store[n], function_str_part) != NULL) {
							head_sw = 1;
							break;
						}
					}
					// 없으면 추가
					if (head_sw == 0) {
						// 만약 헤더가 여러개일 경우, #가 있으면 \n으로 구분해 준다.
						function_str_part = strtok(function_str_part, "#");
						strcpy(header_store[header_count], "#");
						strcat(header_store[header_count], function_str_part);

						strcpy(header_store_c[header_count_c], header_store[header_count]);

						while((function_str_part = strtok(NULL, "#")) != NULL) {
							header_count++;
							header_count_c++;
							strcpy(header_store[header_count], "#");
							strcat(header_store[header_count], function_str_part);
							
							strcpy(header_store_c[header_count_c], header_store[header_count]);
						}
#ifdef DEBUG
						printf("헤더 파일 추가!\n");
						printf("%d 번째 추가한 헤더 파일 : %s\n", header_count+1, header_store[header_count]);
						//	if (header_count > 0) printf("이전의 헤더 파일 : %s\n", header_store[header_count-1]);
#endif
						header_count++; header_count_c++;
						break;
					}
				}

				if (done == 1) break;
			}

		}
		fclose(fp_headertable);

#ifdef DEBUG
		printf("생성해야 하는 헤더 파일\n");
		for (k = 0; k < header_count; k++) {
			printf("%d 번째 : %s\n",k+1, header_store[k]);
		}

#endif
		// 헤더 파일을 생성 - 파일에 쓰기
		for (k = 0; k < header_count; k++) {
			fprintf(fp_result, "%s\n", header_store[k]);
#ifdef DEBUG
			int new_line = 1;
			// -c 옵션
			printf("%d %s\n", new_line++, header_store[k]);	
#endif
		}

		// 본문을 쓰자
		int correct_flag = 0;
		for (k = 0; k < class_count; k++) {
			// 단 {와 }의 개수를 맞출 필요가 있다.
			// 만약 {의 개수 - }의 개수 < 0이면 그 밑부분은 버린다.
			if (strstr(transfer_buf[k], "{") != 0) correct_flag++;
			else if (strstr(transfer_buf[k], "}") != 0) correct_flag--;

			if (correct_flag < 0) break;

			fprintf(fp_result, "%s", transfer_buf[k]);
		}

		class_count_temp = class_count;
		class_count = k;
		fclose(fp_result);


		// 다음으로, main부분 파일을 열어서 써 보자!
		fp_result = fopen(fd_file_tf, "r+");
		fp_headertable = fopen("headertable.txt", "r");

		header_count = 0;
		// 헤더 파일을 생성 - 헤더 파일 찾기
		while(1) {
#ifdef DEBUG
			printf("while문 시작!\n");
#endif
			// 한 줄씩 읽음l
			if (fgets(function_str, sizeof(function_str), fp_headertable) == NULL) {
#ifdef DEBUG
				printf("헤더 테이블을 다 읽음!\n");
#endif
				break;
			}
			// 원본 보관
			strcpy(str_save, function_str);
			function_str_part = strtok(function_str, " "); // open read write close exit printf scanf ...
			strcat(function_str_part, "(");	// printf나 scanf 안에 문자열로 들어간건 아니므로, (를 붙여서 함수인지 검증
			int done = 0; // 특정 함수를 찾았을 경우 1
			// 특정 함수가 있는지 확인, 있으면 그 함수에 맞는 헤더 파일을 넣음
			for (k = class_count_temp; k < transfer_line && done == 0; k++) {
#ifdef DEBUG
	//			printf("특정함수 확인 : 확인할 함수 : %s\n", function_str_part);
	//			printf("검사할 대상 : %s\n", transfer_buf[k]);
	//			printf("검사결과 : %s\n", strstr(transfer_buf[k], function_str_part));
	//			printf("검사결과 : %s\n", strstr(function_str_part, transfer_buf[k]));
#endif
				if (strstr(transfer_buf[k], function_str_part) != NULL) {
					// 만약, open, read, write, close, printf의 경우
					// fopen, fread, fwrite, fclose, fprintf에서 검출된 것인지 확인
					// 만약 그렇다면 continue로 무시
					// 다시 말해서, 함수가 fopen인데 open의 헤더 파일이 들어가는 것을 방지
					char f_str_part[1024] = "f";
					strcat(f_str_part, function_str_part);
					if (strstr(transfer_buf[k], f_str_part) != NULL) {
						continue;
					}

					// 옆에 있는 헤더를 넣어야 하는데, 이게 있는지를 살펴봄
#ifdef DEBUG
					printf("특정함수 발견! 발견된 함수는 %s\n", function_str_part);
#endif
					done = 1;
					int head_sw = 0;
					function_str_part = strtok(str_save, " ");
					function_str_part = strtok(NULL, "\n\0"); // #include <stdio.h> ...
					for (int n = 0;	n < header_count; n++) {
						if (strstr(header_store[n], function_str_part) != NULL) {
							head_sw = 1;
							break;
						}
					}
					// 없으면 추가
					if (head_sw == 0) {
						// 만약 헤더가 여러개일 경우, #가 있으면 \n으로 구분해 준다.
						function_str_part = strtok(function_str_part, "#");
						strcpy(header_store[header_count], "#");
						strcat(header_store[header_count], function_str_part);

						strcpy(header_store_main[header_count_main], header_store[header_count]);

						while((function_str_part = strtok(NULL, "#")) != NULL) {
							header_count++; header_count_main++;
							strcpy(header_store[header_count], "#");
							strcat(header_store[header_count], function_str_part);
							
							strcpy(header_store_main[header_count_main], header_store[header_count]);
						}
#ifdef DEBUG
						printf("헤더 파일 추가!\n");
						printf("%d 번째 추가한 헤더 파일 : %s\n", header_count+1, header_store[header_count]);
						//	if (header_count > 0) printf("이전의 헤더 파일 : %s\n", header_store[header_count-1]);
#endif
						header_count++; header_count_main++;
						break;
					}
				}

				if (done == 1) break;
			}
		}
		fclose(fp_headertable);

#ifdef DEBUG
		printf("생성해야 하는 헤더 파일\n");
		for (k = 0; k < header_count; k++) {
			printf("%d 번째 : %s\n",k+1, header_store[k]);
		}

#endif
		// 헤더 파일을 생성 - 파일에 쓰기
		for (k = 0; k < header_count; k++) {
			fprintf(fp_result, "%s\n", header_store[k]);
#ifdef DEBUG
			int new_line = 1;
			// -c 옵션
			printf("%d %s\n", new_line++, header_store[k]);	
#endif
		}

		// 본문을 쓰자
		correct_flag = 0;
		for (k = class_count_temp; k < transfer_line; k++) {
			// 마찬가지로 {와 }의 개수를 맞춰보자
			// 만약 {의 개수 -}의 개수 < 0이면 그 밑부분은 버린다.
			if (strstr(transfer_buf[k], "{") != 0) correct_flag++;
			else if (strstr(transfer_buf[k], "}") != 0) correct_flag--;

			if (correct_flag < 0) break;

			fprintf(fp_result, "%s", transfer_buf[k]);
		}
		fclose(fp_result);
		
		main_count = k - class_count_temp;	

		// 파일명_Makefile도 함께 생성하기(변환된 C 언어 파일을 컴파일할 수 있어야 함)
		// 실행은 make 파일명 (make q2) 으로 가능
		char m_original[1024];
		strcpy(m_original, makefile_name);

		strcat(makefile_name, "_Makefile");
		FILE *fd_makefile;
		char makefile_temp[1024];
		fd_makefile = fopen(makefile_name, "w+");
		fprintf(fd_makefile, "CC = gcc\n"); 
		fprintf(fd_makefile, "OBJECTS = %s.o %s.o\n", m_original, makefile_name_c); // q2.o Stack.o에서 각각 q2와 Stack
		fprintf(fd_makefile, "TARGET = %s.out\n\n", m_original);
		fprintf(fd_makefile, ".SUFFIXES : .c .o\n\n");
		fprintf(fd_makefile, "all : $(TARGET)\n\n");
		fprintf(fd_makefile, "$(TARGET): $(OBJECTS)\n");
		fprintf(fd_makefile, "	$(CC) -o $@ $(OBJECTS)\n\n");
		fprintf(fd_makefile, "clean:\n");
		fprintf(fd_makefile, "	rm -f $(OBJECTS) $(TARGETS)");
		fclose(fd_makefile);	


#ifdef DEBUG
		printf("%s의 줄수 : %d\n", fd_file_tf_c , class_count);
		printf("%s의 줄수 : %d\n", fd_file_tf, main_count);
	//	exit(0);
#endif

	}

	// 밑의 옵션처리도, 1,3의 경우와 2의 경우는 서로 다르다. 따라서 이 경우 분기를 시켜 준다.
	// r 옵션의 경우 위의 과정을 실행하면서 할 수는 없을까? 고민해보자!


	// 옵션처리를 해 준다. 단, r옵션이 먼저 출력될 수 있게 한다.
	if (r_flag == 1) {

	}

	// 나머지 순서는, j옵션 -> c옵션 -> p옵션 -> f옵션 -> l옵션의 순서로 출력하였다.
	if (j_flag == 1) {
		// j옵션 : 변환할 java 언어 프로그램 코드 출력
		printf("\n\n< %s >\n\n", argv[1]);
		for (i = 0; i < line_count; i++) {
			printf("%d %s", i+1, buffer[i]); // %s에 이미 \n이 있음
		}
		printf("------------------------------------------------------\n");	
	}

	if (c_flag == 1) {
		if (class_count == 0) {
			int new_line = 1;
			// c옵션 : 변환된 C언어 프로그램 출력
			printf("\n\n< %s >\n\n", fd_file_tf);
			// 헤더부분 출력
			for (k = 0; k < header_count; k++) {
				printf("%d %s\n", new_line++, header_store[k]);
			}
			// 함수부분 출력
			for (k = 0; k < transfer_line; k++) {
				printf("%d %s", new_line++, transfer_buf[k]);
			}
		}
		else {
			// class 파일 출력
			printf("\n\n< %s >\n\n", fd_file_tf_c);
			int new_line = 1;
			for (k = 0; k < header_count_c; k++) {
				printf("%d %s\n", new_line++, header_store_c[k]);
			}	
			for (k = 0; k < class_count; k++) {
				printf("%d %s", new_line++, transfer_buf[k]);
			}
			printf("\n\n< %s >\n\n", fd_file_tf);
			// main 파일 출력
			new_line = 1;
			for (k = 0; k < header_count_main; k++) {
				printf("%d %s\n", new_line++, header_store_main[k]);
			}
			for (k = class_count_temp; k < class_count_temp + main_count; k++) {
				printf("%d %s", new_line++, transfer_buf[k]);
			}
		}
		printf("\n------------------------------------------------------\n");
	}

	if (p_flag == 1) {
		// p옵션 : java 언어 프로그램에서 사용된 함수들을 C언어 프로그램에서 대응되는 함수와 함께 출력
		for (k = 0; k < p_count; k++) {
			printf("%d %s\n", k+1, p_buffer[k]);
		}
		printf("------------------------------------------------------\n");
		
	}

	if (f_flag == 1) {
		// f옵션 : java 언어 프로그램 파일 및 C언어 프로그램 파일의 파일 크기 출력
		// 주의 : q1.c처럼 하나의 프로그램만 생성될 수 있지만, q2.c, stack.c 이렇게 두 개가 생성될 수도 있음

		// stat 구조체 생성
		struct stat statbuf;
		stat(argv[1], &statbuf);
		printf("%s file size is %ld bytes\n", argv[1], statbuf.st_size);

		if (class_count != 0) {
			stat(fd_file_tf_c, &statbuf);
			printf("%s file size is %ld bytes\n", fd_file_tf_c, statbuf.st_size);
		}

		stat(fd_file_tf, &statbuf);
		printf("%s file size is %ld bytes\n", fd_file_tf, statbuf.st_size);
		printf("------------------------------------------------------\n");
	}

	if (l_flag == 1) {
		// l옵션 : Java 언어 프로그램 파일 및 C언어 프로그램 파일의 라인 수 출력
		printf("%s line number is %d lines\n", argv[1], line_count);
		if (class_count != 0) {
			printf("%s line number is %d lines\n", fd_file_tf_c, header_count_c + class_count);
			printf("%s line number is %d lines\n", fd_file_tf, header_count_main + main_count);
		}
		else
			printf("%s line number is %d lines\n", fd_file_tf, header_count + transfer_line);
		printf("-----------------------------------------------------\n");
	}	

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	exit(0);
}
