#include <stdio.h>
#include <stdlib.h>  	// exit(0) 사용 가능
#include <ctype.h>      // isdigit(), isalpha(), isspace() 사용 가능
#include <string.h>     // 문자열 함수 사용 가능

//#define DEBUG_TOKEN
//#define DEBUG

char nextChar;
FILE *in_fp;

// nextChar가 뭔지 판단해 주는 함수
int judge(char nextChar);

// 네번째 연산자에 대해 파싱
// 4. ||, &&, |, &을 찾는다. 만약 존재한다면, 좌우로 같은지를 판단한다.
// 단, 왼쪽으로 판단해 나갈 때 (의 개수가 더 많은 지점에서 중지한다.
// 오른쪽으로 판단해 나갈 때는 )의 개수가 더 많은 지점에서 중지한다.
// 괄호로 둘러싸이지 않은 상황에서 ,를 만나면 중지한다.

// 한계 : ,a | b | c | d, 가 있을 때
// (b | c | d ) | a (가능)
// (c | d) | (a | b) (가능)
// d | (a | b | c ) (가능)
//
// c | b | a | d (불가능)
// d | c | b | a (불가능)

int fourth_parsing(int correct_sw_token, int token_count, int token_count_student, char buf_token[][500], char buf_token2[][500]) {
	if (correct_sw_token == 0) {

		char left_stack[500];
		char right_stack[500];
		left_stack[0] = '\0';
		right_stack[0] = '\0';

		// 정답 토큰 개수 : token_count (개)
		int i_token;
		for (i_token = 0; i_token < token_count; i_token++) {
			printf("buf_token2[%d] = %s\n", i_token, buf_token2[i_token]);

			if ((strcmp(buf_token2[i_token], "||") == 0) || (strcmp(buf_token2[i_token], "&&") == 0) ||
				(strcmp(buf_token2[i_token], "|") == 0) || (strcmp(buf_token2[i_token], "&") == 0)) {

				printf("제 4순위 연산자를 찾음!\n");

				// 초기화
				left_stack[0] = '\0';
				right_stack[0] = '\0';


				int j_token = i_token - 1;
				int left_paren = 0;
				int right_paren = 0;

				// 왼쪽 판단
				for (j_token = i_token - 1; j_token > -1; j_token--) {

					// 문자열 분리 고민해보기!
					// 왼쪽 괄호인가?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						// 숫자가 더 많으면 break
						// 아니어도 count 감소
						left_paren++;
						if (left_paren > right_paren) break;
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// 오른쪽 괄호인가?
						right_paren++;
					}
					else if (strcmp(buf_token2[j_token], ",") == 0) {
						// 만약 괄호로 둘러싸이지 않은 상황에서 ,를 만나면 break
						if (left_paren >= right_paren) {
							j_token++;
							break;
						}
					}

					// 
					// strcat(left_stack, buf_token2[j_token]);
					// debug
					// printf("왼쪽 판단 : %s\n", left_stack);
				}

				// 왼쪽 판단의 경우, 위의 strcat를 할 경우 반대로 입력값을 받음!
				// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
				// 이 경우, 맨 뒤에 있는 거부터 역순으로 다시 쓰면 다음과 같음
				// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
				// 따라서, 앞에서부터 붙여넣자
				int left_token_offset_count;
				for (left_token_offset_count = j_token; left_token_offset_count <= i_token - 1; left_token_offset_count++) {
					strcat(left_stack, buf_token2[left_token_offset_count]);
				}

				// debug
				printf("정답 토큰 왼쪽 판단 : %s\n", left_stack);
				//	exit(0);

				j_token = i_token;
				left_paren = 0;
				right_paren = 0;

				// 오른쪽 판단
				for (j_token = i_token + 1; j_token < token_count; j_token++) {

					// 왼쪽 괄호인가?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						left_paren++;
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// 오른쪽 괄호인가?
						// 숫자가 더 많으면 break
						// 아니어도 count 감소
						right_paren++;
						if (right_paren > left_paren) break;
					}
					else if (strcmp(buf_token2[j_token], ",") == 0) {
						// 괄호로 둘러싸이지 않은 상황에서 ,를 만나면 break
						if (right_paren >= left_paren) {
							j_token--;
							break;
						}
					}

					strcat(right_stack, buf_token2[j_token]);
					// debug
					printf("오른쪽 판단 : %s\n", right_stack);
				}




				// 이거, 학생 파일도 파싱해서 저 부분이 있는지 비교해 보아야 함!
				// 학생 파일 토큰 보관하는 배열 : buf_token[]
				// 학생 파일 토큰 개수 : token_count_student


				// 왼쪽(오른쪽) 토큰 시작 offset, 끝 offset
				int left_token_offset[2] = { 0, 0 };
				int right_token_offset[2] = { 0, 0 };


				char left_stack_stu[500];
				char right_stack_stu[500];
				left_stack_stu[0] = '\0';
				right_stack_stu[0] = '\0';

				int i_token_2 = 0;

				// 학생 파일 각각에 대해 비교를 시작함
				for (i_token_2 = 0; i_token_2 < token_count_student; i_token_2++) {
					printf("buf_token[%d] = %s\n", i_token_2, buf_token[i_token_2]);
					if ((strcmp(buf_token[i_token_2], "||") == 0) || (strcmp(buf_token[i_token_2], "&&") == 0) ||
						(strcmp(buf_token[i_token_2], "|") == 0) || (strcmp(buf_token[i_token_2], "&") == 0)) {

						printf("\n\n학생 파일 파싱 시도!\n");

						printf("제 4순위 연산자를 찾음!\n");

						// 초기화
						left_stack_stu[0] = '\0';
						right_stack_stu[0] = '\0';

						int j_token_2 = i_token_2 - 1;
						//		t_count = 0;
						left_paren = 0;
						right_paren = 0;

						// 왼쪽 판단
						for (j_token_2 = i_token_2 - 1; j_token_2 > -1; j_token_2--) {

							// 문자열 분리 고민해보기!
							// 왼쪽 괄호인가?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								// 숫자가 더 많으면 break
								// 아니어도 count 감소
								left_paren++;
								if (left_paren > right_paren) {
									j_token_2++;
									break;
								}
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// 오른쪽 괄호인가?
								right_paren++;
							}
							else if (strcmp(buf_token[j_token_2], ",") == 0 || strcmp(buf_token[j_token_2], "=") == 0) {
								// 만약 괄호로 둘러싸이지 않은 상황에서 , 또는 =를 만나면 break (연산자 우선순위에 따름)
								if (left_paren >= right_paren) {
									j_token_2++;
									break;
								}
							}

							// 
							//strcat(left_stack_stu, buf_token[j_token_2]);
							// debug
							//printf("학생 토큰 왼쪽 판단 : %s\n", left_stack_stu);
						}

						// 왼쪽 판단의 경우, 위의 strcat를 할 경우 반대로 입력값을 받음!
						// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
						// 이 경우, 맨 뒤에 있는 거부터 역순으로 다시 쓰면 다음과 같음
						// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
						// 따라서, 앞에서부터 붙여넣자
						int left_token_offset_count;
						for (left_token_offset_count = j_token_2; left_token_offset_count <= i_token_2 - 1; left_token_offset_count++) {
							strcat(left_stack_stu, buf_token[left_token_offset_count]);
						}

						// debug
						printf("학생 토큰 왼쪽 판단 : %s\n", left_stack_stu);
						//	exit(0);

						// 왼쪽 토큰 offset 위치시킴
						left_token_offset[1] = i_token_2 - 1;
						left_token_offset[0] = j_token_2;


						j_token_2 = i_token_2;
						left_paren = 0;
						right_paren = 0;

						// 오른쪽 판단
						for (j_token_2 = i_token_2 + 1; j_token_2 < token_count_student; j_token_2++) {

							// 왼쪽 괄호인가?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								left_paren++;
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// 오른쪽 괄호인가?
								right_paren++;
								if (right_paren > left_paren) {
									j_token_2--;
									break;
								}
							}
							else if (strcmp(buf_token[j_token_2], ",") == 0 || strcmp(buf_token[j_token_2], "=") == 0) {
								// 만약 괄호로 둘러싸이지 않은 상황에서 , 또는 =를 만나면 break (연산자 우선순위에 따름)
								if (right_paren >= left_paren) {
									j_token_2--;
									break;
								}
							}

							strcat(right_stack_stu, buf_token[j_token_2]);
							// debug
							printf("j_token_2 = %d, token_count_student = %d\n", j_token_2, token_count_student);
							printf("오른쪽 판단 : %s\n", right_stack_stu);
						}


						// 오른쪽 토큰 offset 위치시킴
						right_token_offset[0] = i_token_2 + 1;
						right_token_offset[1] = j_token_2;


						// 정답 파일과 학생 파일 대조, 만약 맞으면 토큰을 이동시킴
						// 	
						// 	정답 토큰 왼쪽 : left_stack
						// 	정답 토큰 오른쪽 : right_stack
						// 	학생 토큰 왼쪽 : left_stack_stu
						// 	학생 토큰 오른쪽 : right_stack_stu
						//
						// 정답 토큰 왼쪽 == 학생 토큰 오른쪽?
						// 정답 토큰 오른쪽 == 학생 토큰 왼쪽?
						// 이 둘이 맞으면, 학생 토큰 두 부분을 옮기고 다시 토큰별로 비교함
						// 성공하면 true를 내고 밑으로 내려가고, 실패시 continue로 반복

						char token_temp_part[100][500];

						printf("정답 토큰 왼쪽 : %s, 정답 토큰 오른쪽 : %s\n", left_stack, right_stack);
						printf("학생 토큰 왼쪽 : %s, 학생 토큰 오른쪽 : %s\n", left_stack_stu, right_stack_stu);

						printf("학생 토큰의 범위 - 왼쪽: (left_token_offset[0]) ~ (left_token_offset[1]) : %d ~ %d\n", left_token_offset[0], left_token_offset[1]);

						printf("학생 토큰의 범위 - 오른쪽: (right_token_offset[0]) ~ (right_token_offset[1]) : %d ~ %d\n", right_token_offset[0], right_token_offset[1]);


						if (strcmp(left_stack, right_stack_stu) == 0 && strcmp(right_stack, left_stack_stu) == 0) {

							printf("다음 연산자인 %s 사이에 있는 둘을 바꿔 봅시다!\n", buf_token[left_token_offset[1] + 1]);

							// 일단 right_token_offset[0] 부분부터 right_token_offset[1] 부분의 학생 토큰(buf_token[])을 발췌함
							int i_change_token = 0;
							int i_zero_start = 0;
							for (i_change_token = right_token_offset[0]; i_change_token <= right_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}
							// ||나 | 또는 &&따위의 연산자를 넣음
							strcpy(token_temp_part[i_zero_start], buf_token[left_token_offset[1] + 1]);

							// left_token_offset[0] 부분부터 left_token_offset[1] 부분의 학생 토큰(buf_token[])을 발췌함
							i_change_token = 0;
							i_zero_start++;
							for (i_change_token = left_token_offset[0]; i_change_token <= left_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}

							int i_zero_start2 = 0;
							// DEBUG
							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								printf("\n바뀐 토큰 출력해 보기!\n\n");
								printf("token_temp_part[%d] = %s\n", i_zero_start2, token_temp_part[i_zero_start2]);
							}

							// token_temp_part에 있는 토큰들을 학생 토큰 위치에 옮김
							i_zero_start2 = 0;
							i_change_token = left_token_offset[0];
							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								strcpy(buf_token[i_change_token], token_temp_part[i_zero_start2]);
								i_change_token++;
							}

							// DEBUG
							printf("\n\n바뀐 학생 토큰을 출력해 봅시다!\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count_student; i_zero_start2++) {
								printf("buf_token[%d] = %s\n", i_zero_start2, buf_token[i_zero_start2]);
							}

							// DEBUG
							printf("\n\n정답 토큰을 다시 출력해 볼까요?\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								printf("buf_token2[%d] = %s\n", i_zero_start2, buf_token2[i_zero_start2]);
							}

							// 학생 토큰과 정답 토큰이 모두 같다면 정답입니다 출력
							// 그렇지 않을 경우 4.에서 그래도 실패 출력
							int one_cond_flag = 1; // 첫 번째 조건이 만족되었을 경우 1
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								if (strcmp(buf_token[i_zero_start2], buf_token2[i_zero_start2]) != 0) {
									one_cond_flag = 0;
									printf("4번째 연산자 비교 그래도 실패!\n");
									break;
								}
							}

							if (one_cond_flag == 1) {
								printf("정답입니다!\n");
								return 1;
							}

							//	exit(1);
						}

						// 실패하면 그냥 밑으로 쭉 내려감 (다음 과정 비교 시작)
						// 성공하면(same판정 뜨면) 밑에 꺼 쭉 내려가서, 정답 처리됨


					}
				}
			}

			printf("4번이 끝났나요?\n");
			printf("틀림!\n");
		}
	}
	else {

		printf("맞음!\n");
		return 1;
	}
	printf("\n\n한 정답 파일에 대해 판정이 끝남!!!\n\n\n");
	printf("최종 실패!\n");
	return 0;
}

// 3. ==, !=을 찾는다. 만약 존재한다면, 좌우로 같은지를 판단한다.
// 단, 왼쪽으로 판단해 나갈 때 (의 개수가 더 많은 지점에서 중지한다.
// 오른쪽으로 판단해 나갈 때는 )의 개수가 더 많은 지점에서 중지한다.
// 괄호로 둘러싸이지 않은 상황에서 , 를 만나면 중지한다.
// 괄호 안에 있지 않은 상황에서 4번 연산자(||, &&, |, &)들을 만나면 중지한다.

int third_parsing(int correct_sw_token, int token_count, int token_count_student, char buf_token[][500], char buf_token2[][500])
{

	if (correct_sw_token == 0) {

		char left_stack_db[500][500];	// left_stack의 토큰 보관
		char right_stack_db[500][500];	// right_stack의 토큰 보관

		char left_stack_db_count = 0;	// left_stack의 토큰 개수
		char right_stack_db_count = 0;	// right_stack의 토큰 개수

		char left_stack[500];
		char right_stack[500];
		left_stack[0] = '\0';
		right_stack[0] = '\0';

		// 정답 토큰 개수 : token_count (개)
		int i_token;
		for (i_token = 0; i_token < token_count; i_token++) {
			printf("buf_token2[%d] = %s\n", i_token, buf_token2[i_token]);

			if ((strcmp(buf_token2[i_token], "==") == 0) || (strcmp(buf_token2[i_token], "!=") == 0)) {

				printf("제 3순위 연산자 == 또는 != 를 찾음!\n");

				// 초기화
				left_stack[0] = '\0';
				right_stack[0] = '\0';


				int j_token = i_token - 1;
				int left_paren = 0;
				int right_paren = 0;

				// 왼쪽 판단
				for (j_token = i_token - 1; j_token > -1; j_token--) {

					// 문자열 분리 고민해보기!
					// 왼쪽 괄호인가?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						// 숫자가 더 많으면 break
						// 아니어도 count 감소
						left_paren++;
						if (left_paren > right_paren) {
							j_token++;
							break;
						}
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// 오른쪽 괄호인가?
						right_paren++;
					}
					else if ((strcmp(buf_token2[j_token], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], "=") == 0)) {
						// 만약 괄호로 둘러싸이지 않은 상황에서 , 또는 4번 연산자(||, &&, |, &)를 만나면 break
						if (left_paren >= right_paren) {
							j_token++;
							break;
						}
					}

					// 
					//	strcat(left_stack, buf_token2[j_token]);
					// debug
					//	printf("왼쪽 판단 : %s\n", left_stack);
				}

				// 왼쪽 판단의 경우, 위의 strcat를 할 경우 반대로 입력값을 받음!
				// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
				// 이 경우, 맨 뒤에 있는 거부터 역순으로 다시 쓰면 다음과 같음
				// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
				// 따라서, 앞에서부터 붙여넣자
				int left_token_offset_count;
				left_stack_db_count = 0;
				for (left_token_offset_count = j_token; left_token_offset_count <= i_token - 1; left_token_offset_count++) {
					strcat(left_stack, buf_token2[left_token_offset_count]);
					strcpy(left_stack_db[left_token_offset_count - j_token], buf_token2[left_token_offset_count]);
					left_stack_db_count++;
				}

				// debug
				printf("정답 토큰 왼쪽 판단 : %s\n", left_stack);
				//	exit(0);



				j_token = i_token;
				left_paren = 0;
				right_paren = 0;

				// 오른쪽 판단
				for (j_token = i_token + 1; j_token < token_count; j_token++) {

					// 왼쪽 괄호인가?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						left_paren++;
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// 오른쪽 괄호인가?
						// 숫자가 더 많으면 break
						// 아니어도 count 감소
						right_paren++;
						if (right_paren > left_paren) {
							j_token--;
							break;
						}
					}
					else if ((strcmp(buf_token2[j_token], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], "=") == 0)) {
						// 만약 괄호로 둘러싸이지 않은 상황에서 , 또는 4번 연산자(||, &&, |, &) 또한 = 를 만나면 break
						if (right_paren >= left_paren) {
							j_token--;
							break;
						}
					}

					strcat(right_stack, buf_token2[j_token]);
					strcpy(right_stack_db[right_stack_db_count], buf_token2[j_token]);
					right_stack_db_count++;
					// debug
					printf("오른쪽 판단 : %s\n", right_stack);

				}




				// 이거, 학생 파일도 파싱해서 저 부분이 있는지 비교해 보아야 함!
				// 학생 파일 토큰 보관하는 배열 : buf_token[]
				// 학생 파일 토큰 개수 : token_count_student


				// 왼쪽(오른쪽) 토큰 시작 offset, 끝 offset
				int left_token_offset[2] = { 0, 0 };
				int right_token_offset[2] = { 0, 0 };
				int left_stack_stu_db_count = 0;
				int right_stack_stu_db_count = 0;
				char left_stack_stu_db[500][500];
				char right_stack_stu_db[500][500];
				char left_stack_stu[500];
				char right_stack_stu[500];
				left_stack_stu[0] = '\0';
				right_stack_stu[0] = '\0';

				int i_token_2 = 0;

				// 학생 파일 각각에 대해 비교를 시작함
				for (i_token_2 = 0; i_token_2 < token_count_student; i_token_2++) {
					printf("buf_token[%d] = %s\n", i_token_2, buf_token[i_token_2]);
					if ((strcmp(buf_token[i_token_2], "==") == 0) || (strcmp(buf_token[i_token_2], "!=") == 0)) {

						printf("\n\n학생 파일 파싱 시도!\n");

						printf("제 3순위 연산자를 찾음!\n");

						// 초기화
						left_stack_stu[0] = '\0';
						right_stack_stu[0] = '\0';

						int j_token_2 = i_token_2 - 1;
						left_paren = 0;
						right_paren = 0;

						// 왼쪽 판단
						for (j_token_2 = i_token_2 - 1; j_token_2 > -1; j_token_2--) {

							// 문자열 분리 고민해보기!
							// 왼쪽 괄호인가?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								// 숫자가 더 많으면 break
								// 아니어도 count 감소
								left_paren++;
								if (left_paren > right_paren) {
									j_token_2++;
									break;
								}
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// 오른쪽 괄호인가?
								right_paren++;
							}
							else if ((strcmp(buf_token2[j_token], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], ",") == 0)) {
								// 만약 괄호로 둘러싸이지 않은 상황에서, 또는 4번 연산자(||, &&, |, &) 또는 = 를 만나면 break
								if (left_paren >= right_paren) {
									j_token_2++;
									break;
								}
							}

							// 
							//strcat(left_stack_stu, buf_token[j_token_2]);
							// debug
							//printf("학생 토큰 왼쪽 판단 : %s\n", left_stack_stu);


						}


						// 왼쪽 판단의 경우, 위의 strcat를 할 경우 반대로 입력값을 받음!
						// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
						// 이 경우, 맨 뒤에 있는 거부터 역순으로 다시 쓰면 다음과 같음
						// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
						// 따라서, 앞에서부터 붙여넣자
						left_stack_stu_db_count = 0;
						int left_token_offset_count;
						for (left_token_offset_count = j_token_2; left_token_offset_count <= i_token_2 - 1; left_token_offset_count++) {
							strcat(left_stack_stu, buf_token[left_token_offset_count]);
							strcpy(left_stack_stu_db[left_token_offset_count - j_token_2], buf_token[left_token_offset_count]);
							left_stack_stu_db_count++;
						}

						// debug
						printf("학생 토큰 왼쪽 판단 : %s\n", left_stack_stu);
						//	exit(0);

						// 왼쪽 토큰 offset 위치시킴
						left_token_offset[1] = i_token_2 - 1;
						left_token_offset[0] = j_token_2;


						j_token_2 = i_token_2;
						left_paren = 0;
						right_paren = 0;

						// 오른쪽 판단
						for (j_token_2 = i_token_2 + 1; j_token_2 < token_count_student; j_token_2++) {

							// 왼쪽 괄호인가?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								left_paren++;
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// 오른쪽 괄호인가?
								right_paren++;
								if (right_paren > left_paren) {
									j_token_2--;
									break;
								}
							}
							else if ((strcmp(buf_token[j_token_2], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], "=") == 0)) {
								// 괄호로 둘러싸이지 않은 상황에서 , 또는 4번 연산자(||, &&, |, &) 또는 = 를 만나면 break

								if (right_paren >= left_paren) {
									j_token_2--;
									break;
								}
							}

							strcat(right_stack_stu, buf_token[j_token_2]);
							strcpy(right_stack_stu_db[right_stack_db_count], buf_token[j_token_2]);
							right_stack_db_count++;
							// debug
							printf("오른쪽 판단 : %s\n", right_stack_stu);
						}


						// 오른쪽 토큰 offset 위치시킴
						right_token_offset[0] = i_token_2 + 1;
						right_token_offset[1] = j_token_2;


						// 정답 파일과 학생 파일 대조, 만약 맞으면 토큰을 이동시킴
						// 	
						// 	정답 토큰 왼쪽 : left_stack
						// 	정답 토큰 오른쪽 : right_stack
						// 	학생 토큰 왼쪽 : left_stack_stu
						// 	학생 토큰 오른쪽 : right_stack_stu
						//
						// 정답 토큰 왼쪽 == 학생 토큰 오른쪽?
						// 정답 토큰 오른쪽 == 학생 토큰 왼쪽?
						// 이 둘이 맞으면, 학생 토큰 두 부분을 옮기고 다시 토큰별로 비교함
						// 성공하면 true를 내고 밑으로 내려가고, 실패시 continue로 반복

						char token_temp_part[100][500];

						printf("3순위 연산자 기준, 발견된 3순위 연산자는 %s\n", buf_token[left_token_offset[1] + 1]);
						printf("정답 토큰 왼쪽 : %s, 정답 토큰 오른쪽 : %s\n", left_stack, right_stack);
						printf("학생 토큰 왼쪽 : %s, 학생 토큰 오른쪽 : %s\n", left_stack_stu, right_stack_stu);

						printf("학생 토큰의 범위 - 왼쪽: (left_token_offset[0]) ~ (left_token_offset[1]) : %d ~ %d\n", left_token_offset[0], left_token_offset[1]);

						printf("학생 토큰의 범위 - 오른쪽: (right_token_offset[0]) ~ (right_token_offset[1]) : %d ~ %d\n", right_token_offset[0], right_token_offset[1]);


						// 만약 두 토큰이 같지 않은 경우, 하위 연산자 (&& || & |)에 대한 파싱을 시도해 본다.
						if (strcmp(left_stack, right_stack_stu) != 0) {
							// 한쪽만 파싱한 뒤 맞는지 판정하면 될 듯!
							// 4번째 함수 불러오기
							fourth_parsing(0, left_stack_db_count, right_stack_stu_db_count, left_stack_db, right_stack_stu_db);
						}


						// 만약 두 토큰이 같지 않은 경우, 하위 연산자에 대한 파싱을 시도해 본다.
						if (strcmp(right_stack, left_stack_stu) != 0) {
							// 한쪽만 파싱한 뒤 맞는지 판정하면 될 듯!
							// 4번째 함수 불러오기
							fourth_parsing(0, left_stack_stu_db_count, right_stack_db_count, right_stack_db, left_stack_stu_db);
						}

						if (strcmp(left_stack, right_stack_stu) == 0 && strcmp(right_stack, left_stack_stu) == 0) {

							printf("다음 연산자인 %s 사이에 있는 둘을 바꿔 봅시다!\n", buf_token[left_token_offset[1] + 1]);

							// 일단 right_token_offset[0] 부분부터 right_token_offset[1] 부분의 학생 토큰(buf_token[])을 발췌함
							int i_change_token = 0;
							int i_zero_start = 0;
							for (i_change_token = right_token_offset[0]; i_change_token <= right_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}
							// ==나 != 따위의 연산자를 넣음
							strcpy(token_temp_part[i_zero_start], buf_token[left_token_offset[1] + 1]);

							// left_token_offset[0] 부분부터 left_token_offset[1] 부분의 학생 토큰(buf_token[])을 발췌함
							i_change_token = 0;
							i_zero_start++;
							for (i_change_token = left_token_offset[0]; i_change_token <= left_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}

							int i_zero_start2 = 0;
							// DEBUG

							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								printf("\n바뀐 토큰 출력해 보기!\n\n");
								printf("token_temp_part[%d] = %s\n", i_zero_start2, token_temp_part[i_zero_start2]);
							}

							// token_temp_part에 있는 토큰들을 학생 토큰 위치에 옮김
							i_zero_start2 = 0;
							i_change_token = left_token_offset[0];
							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								strcpy(buf_token[i_change_token], token_temp_part[i_zero_start2]);
								i_change_token++;
							}

							// DEBUG
							printf("\n\n바뀐 학생 토큰을 출력해 봅시다!\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count_student; i_zero_start2++) {
								printf("buf_token[%d] = %s\n", i_zero_start2, buf_token[i_zero_start2]);
							}

							// DEBUG
							printf("\n\n정답 토큰을 다시 출력해 볼까요?\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								printf("buf_token2[%d] = %s\n", i_zero_start2, buf_token2[i_zero_start2]);
							}

							// 학생 토큰과 정답 토큰이 모두 같다면 정답입니다 출력
							// 그렇지 않을 경우 1.에서 그래도 실패 출력
							int one_cond_flag = 1; // 첫 번째 조건이 만족되었을 경우 1
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								if (strcmp(buf_token[i_zero_start2], buf_token2[i_zero_start2]) != 0) {
									one_cond_flag = 0;
									printf("1. 에서 그래도 실패!\n");
									break;
								}
							}

							if (one_cond_flag == 1) {
								printf("정답입니다!\n");
								return 1;
							}

							//	exit(1);
						}

						// 실패하면 그냥 밑으로 쭉 내려감 (다음 과정 비교 시작)
						// 성공하면(same판정 뜨면) 밑에 꺼 쭉 내려가서, 정답 처리됨

					}

				}

				printf("정답 파일 파싱을 완료하였습니다!\n");

			}

			printf("3번이 끝났나요?\n");
			printf("틀림!\n");
		}
	}
	else {
		printf("맞음!\n");
		return 1;
	}

	printf("\n\n하나 판정이 끝남!!!\n\n\n");
	// debug
	// exit(1);

	//	return;
	printf("세번째 거 끝남!!\n\n");
	return fourth_parsing(correct_sw_token, token_count, token_count_student, buf_token, buf_token2);
}



// 2. <, >, <=, >=을 찾는다. 만약 존재한다면, 좌우로 같은지를 판단한다.
// 단, 왼쪽으로 판단해 나갈 때 (의 개수가 더 많은 지점에서 중지한다.
// 오른쪽으로 판단해 나갈 때는 )의 개수가 더 많은 지점에서 중지한다.
// 괄호로 둘러싸이지 않은 상황에서 ,를 만나면 중지한다.
// 괄호 안에 있지 않은 상황에서 3번, 4번 연산자들을 만나면 중지한다.
int second_parsing(int correct_sw_token, int token_count, int token_count_student, char buf_token[][500], char buf_token2[][500]) {

	if (correct_sw_token == 0) {

		char left_stack[500];
		char right_stack[500];
		left_stack[0] = '\0';
		right_stack[0] = '\0';

		char left_stack_db[500][500];	// left_stack의 토큰 보관
		char right_stack_db[500][500];	// right_stack의 토큰 보관

		int left_stack_db_count = 0;	// left_stack의 토큰 개수
		int right_stack_db_count = 0;	// right_stack의 토큰 개수
		// 정답 토큰 개수 : token_count (개)

		int i_token;
		for (i_token = 0; i_token < token_count; i_token++) {
			printf("buf_token2[%d] = %s\n", i_token, buf_token2[i_token]);

			if ((strcmp(buf_token2[i_token], "<") == 0) || (strcmp(buf_token2[i_token], ">") == 0)) {

				printf("제 2순위 연산자 < 또는 > 를 찾음!\n");

				// 초기화
				left_stack[0] = '\0';
				right_stack[0] = '\0';


				int j_token = i_token - 1;
				int left_paren = 0;
				int right_paren = 0;

				// 왼쪽 판단
				for (j_token = i_token - 1; j_token > -1; j_token--) {

					// 문자열 분리 고민해보기!
					// 왼쪽 괄호인가?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						// 숫자가 더 많으면 break
						// 아니어도 count 감소
						left_paren++;
						if (left_paren > right_paren) {
							j_token++;
							break;
						}
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// 오른쪽 괄호인가?
						right_paren++;
					}
					else if ((strcmp(buf_token2[j_token], ",") == 0) || ((strcmp(buf_token2[j_token], "==") == 0) || (strcmp(buf_token2[j_token], "!=") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], "=") == 0))) {
						// 만약 괄호로 둘러싸이지 않은 상황에서 , 또는 3번 연산자(==, !=) 또는 4번 연산자(||, &&, |, &)를 만나면 break
						if (left_paren >= right_paren) {
							j_token++;
							break;
						}
					}

					// 
					//	strcat(left_stack, buf_token2[j_token]);
					// debug
					//	printf("왼쪽 판단 : %s\n", left_stack);

				}


				// 왼쪽 판단의 경우, 위의 strcat를 할 경우 반대로 입력값을 받음!
				// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
				// 이 경우, 맨 뒤에 있는 거부터 역순으로 다시 쓰면 다음과 같음
				// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
				// 따라서, 앞에서부터 붙여넣자
				left_stack_db_count = 0;
				int left_token_offset_count;
				for (left_token_offset_count = j_token; left_token_offset_count <= i_token - 1; left_token_offset_count++) {
					strcat(left_stack, buf_token2[left_token_offset_count]);
					strcpy(left_stack_db[left_token_offset_count - j_token], buf_token2[left_token_offset_count]);
					left_stack_db_count++;
				}

				// debug
				printf("정답 토큰 왼쪽 판단 : %s\n", left_stack);
				//	exit(0);



				j_token = i_token;
				left_paren = 0;
				right_paren = 0;
				right_stack_db_count++;
				// 오른쪽 판단
				for (j_token = i_token + 1; j_token < token_count; j_token++) {
					printf("token_count : %d, j_token : %d\n", token_count, j_token);
					// 왼쪽 괄호인가?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						left_paren++;
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// 오른쪽 괄호인가?
						// 숫자가 더 많으면 break
						// 아니어도 count 감소
						right_paren++;
						if (right_paren > left_paren) {
							j_token--;
							break;
						}
					}
					else if ((strcmp(buf_token2[j_token], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || ((strcmp(buf_token2[j_token], "==") == 0) || ((strcmp(buf_token2[j_token], "!=") == 0) || (strcmp(buf_token2[j_token], "=") == 0)))) 
					{
						// 만약 괄호로 둘러싸이지 않은 상황에서 , 또는 3번 연산자(==, !=) 또는 4번 연산자(||, &&, |, &) 또한 = 를 만나면 break
						if (right_paren >= left_paren) {
							j_token--;
							break;
						}
					}

					strcat(right_stack, buf_token2[j_token]);
					strcpy(right_stack_db[right_stack_db_count], buf_token2[j_token]);
					right_stack_db_count++;
					// debug
					printf("오른쪽 판단 : %s\n", right_stack);
				}


			// 이거, 학생 파일도 파싱해서 저 부분이 있는지 비교해 보아야 함!
			// 학생 파일 토큰 보관하는 배열 : buf_token[]
			// 학생 파일 토큰 개수 : token_count_student


			// 왼쪽(오른쪽) 토큰 시작 offset, 끝 offset
			int left_token_offset[2] = { 0, 0 };
			int right_token_offset[2] = { 0, 0 };


			char left_stack_stu_db[500][500];
			char right_stack_stu_db[500][500];
			int left_stack_stu_db_count = 0;
			int right_stack_stu_db_count = 0;
			char left_stack_stu[500];
			char right_stack_stu[500];
			left_stack_stu[0] = '\0';
			right_stack_stu[0] = '\0';

			int i_token_2 = 0;

			// 학생 파일 각각에 대해 비교를 시작함
			for (i_token_2 = 0; i_token_2 < token_count_student; i_token_2++) {
				printf("buf_token[%d] = %s\n", i_token_2, buf_token[i_token_2]);
				if ((strcmp(buf_token[i_token_2], "==") == 0) || (strcmp(buf_token[i_token_2], "!=") == 0)) {
						printf("\n\n학생 파일 파싱 시도!\n");
						printf("제 3순위 연산자를 찾음!\n");

						// 초기화
						left_stack_stu[0] = '\0';
						right_stack_stu[0] = '\0';

						int j_token_2 = i_token_2 - 1;
						left_paren = 0;
						right_paren = 0;

						// 왼쪽 판단
						for (j_token_2 = i_token_2 - 1; j_token_2 > -1; j_token_2--) {

							// 문자열 분리 고민해보기!
							// 왼쪽 괄호인가?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								// 숫자가 더 많으면 break
								// 아니어도 count 감소
								left_paren++;
								if (left_paren > right_paren) {
									j_token_2++;
									break;
								}
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// 오른쪽 괄호인가?
								right_paren++;
							}
							else if ((strcmp(buf_token2[j_token], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], ",") == 0)) {
								// 만약 괄호로 둘러싸이지 않은 상황에서, 또는 4번 연산자(||, &&, |, &) 또는 = 를 만나면 break
								if (left_paren >= right_paren) {
									j_token_2++;
									break;
								}
							}

							// 
							//strcat(left_stack_stu, buf_token[j_token_2]);
							// debug
							//printf("학생 토큰 왼쪽 판단 : %s\n", left_stack_stu);


						}


						// 왼쪽 판단의 경우, 위의 strcat를 할 경우 반대로 입력값을 받음!
						// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
						// 이 경우, 맨 뒤에 있는 거부터 역순으로 다시 쓰면 다음과 같음
						// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
						// 따라서, 앞에서부터 붙여넣자
						left_stack_stu_db_count = 0;
						int left_token_offset_count;
						for (left_token_offset_count = j_token_2; left_token_offset_count <= i_token_2 - 1; left_token_offset_count++) {
							strcat(left_stack_stu, buf_token[left_token_offset_count]);
							strcpy(left_stack_stu_db[left_token_offset_count - j_token_2], buf_token[left_token_offset_count]);
							left_stack_stu_db_count++;
						}

						// debug
						printf("학생 토큰 왼쪽 판단 : %s\n", left_stack_stu);
						//	exit(0);

						// 왼쪽 토큰 offset 위치시킴
						left_token_offset[1] = i_token_2 - 1;
						left_token_offset[0] = j_token_2;


						j_token_2 = i_token_2;
						left_paren = 0;
						right_paren = 0;

						// 오른쪽 판단
						for (j_token_2 = i_token_2 + 1; j_token_2 < token_count_student; j_token_2++) {

							// 왼쪽 괄호인가?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								left_paren++;
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// 오른쪽 괄호인가?
								right_paren++;
								if (right_paren > left_paren) {
									j_token_2--;
									break;
								}
							}
							else if ((strcmp(buf_token[j_token_2], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], "=") == 0)) {
								// 괄호로 둘러싸이지 않은 상황에서 , 또는 4번 연산자(||, &&, |, &) 또는 = 를 만나면 break

								if (right_paren >= left_paren) {
									j_token_2--;
									break;
								}
							}

							strcat(right_stack_stu, buf_token[j_token_2]);
							strcpy(right_stack_stu_db[right_stack_db_count], buf_token[j_token_2]);
							right_stack_db_count++;
							// debug
							printf("오른쪽 판단 : %s\n", right_stack_stu);
						}


						// 오른쪽 토큰 offset 위치시킴
						right_token_offset[0] = i_token_2 + 1;
						right_token_offset[1] = j_token_2;


						// 정답 파일과 학생 파일 대조, 만약 맞으면 토큰을 이동시킴
						// 	
						// 	정답 토큰 왼쪽 : left_stack
						// 	정답 토큰 오른쪽 : right_stack
						// 	학생 토큰 왼쪽 : left_stack_stu
						// 	학생 토큰 오른쪽 : right_stack_stu
						//
						// 정답 토큰 왼쪽 == 학생 토큰 오른쪽?
						// 정답 토큰 오른쪽 == 학생 토큰 왼쪽?
						// 이 둘이 맞으면, 학생 토큰 두 부분을 옮기고 다시 토큰별로 비교함
						// 성공하면 true를 내고 밑으로 내려가고, 실패시 continue로 반복

						char token_temp_part[100][500];

						printf("3순위 연산자 기준, 발견된 3순위 연산자는 %s\n", buf_token[left_token_offset[1] + 1]);
						printf("정답 토큰 왼쪽 : %s, 정답 토큰 오른쪽 : %s\n", left_stack, right_stack);
						printf("학생 토큰 왼쪽 : %s, 학생 토큰 오른쪽 : %s\n", left_stack_stu, right_stack_stu);

						printf("학생 토큰의 범위 - 왼쪽: (left_token_offset[0]) ~ (left_token_offset[1]) : %d ~ %d\n", left_token_offset[0], left_token_offset[1]);

						printf("학생 토큰의 범위 - 오른쪽: (right_token_offset[0]) ~ (right_token_offset[1]) : %d ~ %d\n", right_token_offset[0], right_token_offset[1]);

						// 만약 두 토큰이 같지 않은 경우, 하위 연산자에 대한 파싱을 시도해 본다.
						if (strcmp(left_stack, right_stack_stu) != 0) {

							// 한쪽만 파싱한 뒤 맞는지 판정하면 될 듯!
							// 3번째 함수 불러오기
							third_parsing(0, left_stack_db_count, right_stack_stu_db_count, left_stack_db, right_stack_stu_db);

						}

						// 만약 두 토큰이 같지 않은 경우, 하위 연산자에 대한 파싱을 시도해 본다.
						if (strcmp(right_stack, left_stack_stu) != 0) {
							// 한쪽만 파싱한 뒤 맞는지 판정하면 될 듯!

							// 3번째 함수 불러오기
							third_parsing(0, left_stack_stu_db_count, right_stack_db_count, right_stack_db, left_stack_stu_db);
						}

						if (strcmp(left_stack, right_stack_stu) == 0 && strcmp(right_stack, left_stack_stu) == 0) {

							printf("다음 연산자인 %s 사이에 있는 둘을 바꿔 봅시다!\n", buf_token[left_token_offset[1] + 1]);

							// 일단 right_token_offset[0] 부분부터 right_token_offset[1] 부분의 학생 토큰(buf_token[])을 발췌함
							int i_change_token = 0;
							int i_zero_start = 0;
							for (i_change_token = right_token_offset[0]; i_change_token <= right_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}
							// <면 >를, <=면 >=를, >면 <를, >=면 <= 따위의 연산자를 넣음
							if (strcmp(buf_token[left_token_offset[1] + 1], "<") == 0)
								strcpy(token_temp_part[i_zero_start], ">");
							else if (strcmp(buf_token[left_token_offset[1] + 1], ">") == 0)
								strcpy(token_temp_part[i_zero_start], "<");
							else if (strcmp(buf_token[left_token_offset[1] + 1], ">=") == 0)
								strcpy(token_temp_part[i_zero_start], "<=");
							else if (strcmp(buf_token[left_token_offset[1] + 1], "<=") == 0)
								strcpy(token_temp_part[i_zero_start], ">=");
							else
								printf("연산자가 잘못 입력받아졌습니다!\n");

							// left_token_offset[0] 부분부터 left_token_offset[1] 부분의 학생 토큰(buf_token[])을 발췌함
							i_change_token = 0;
							i_zero_start++;
							for (i_change_token = left_token_offset[0]; i_change_token <= left_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}

							int i_zero_start2 = 0;
							// DEBUG

							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								printf("\n바뀐 토큰 출력해 보기!\n\n");
								printf("token_temp_part[%d] = %s\n", i_zero_start2, token_temp_part[i_zero_start2]);
							}

							// token_temp_part에 있는 토큰들을 학생 토큰 위치에 옮김
							i_zero_start2 = 0;
							i_change_token = left_token_offset[0];
							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								strcpy(buf_token[i_change_token], token_temp_part[i_zero_start2]);
								i_change_token++;
							}

							// DEBUG
							printf("\n\n바뀐 학생 토큰을 출력해 봅시다!\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count_student; i_zero_start2++) {
								printf("buf_token[%d] = %s\n", i_zero_start2, buf_token[i_zero_start2]);
							}

							// DEBUG
							printf("\n\n정답 토큰을 다시 출력해 볼까요?\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								printf("buf_token2[%d] = %s\n", i_zero_start2, buf_token2[i_zero_start2]);
							}

							// 학생 토큰과 정답 토큰이 모두 같다면 정답입니다 출력
							// 그렇지 않을 경우 1.에서 그래도 실패 출력
							int one_cond_flag = 1; // 첫 번째 조건이 만족되었을 경우 1
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								if (strcmp(buf_token[i_zero_start2], buf_token2[i_zero_start2]) != 0) {
									one_cond_flag = 0;
									printf("1. 에서 그래도 실패!\n");
									break;
								}
							}

							if (one_cond_flag == 1) {
								printf("정답입니다!\n");
								return 1;
							}

							//	exit(1);
						}

						// 실패하면 그냥 밑으로 쭉 내려감 (다음 과정 비교 시작)
						// 성공하면(same판정 뜨면) 밑에 꺼 쭉 내려가서, 정답 처리됨

					}

				}

				printf("정답 파일 파싱을 완료하였습니다!\n");

			}

			printf("3번이 끝났나요?\n");
			printf("틀림!\n");
			//return 0;
		}
	}
	else {
		printf("맞음!\n");
		return 1;
	}

	printf("\n\n하나 판정이 끝남!!!\n\n\n");
	// debug
	// exit(1);
	return third_parsing(correct_sw_token, token_count, token_count_student, buf_token, buf_token2);
}


// parsing 함수
int parsing(char *student_txt_path, char *answer_txt_path) {

	int now_flag = 0;  // 지금 상태를 나타냄
	int prev_flag = 0; // 바로 전 상태를 나타냄

	int correct_flag = 0; // 맞았는지 확인

	printf("student_txt_path : %s\n", student_txt_path);
	printf("answer_txt_path : %s\n", answer_txt_path);

	/////////////////////////////////////////////////////////////////////////////////////////////
	//                         학생 답 토큰 분리                                               //
	/////////////////////////////////////////////////////////////////////////////////////////////

	/* Open the input data file and process its contents */
	if ((in_fp = fopen(student_txt_path, "r")) == NULL) {
		printf("ERROR - cannot open front.in \n");
		exit(0);
	}

	// 한 글자씩 입력받자.
	int token_count_student = 0;
	int buf_count = 0;
	char buf_temp[1024];
	buf_temp[0] = '\0';
	char buf_token[500][500];

	while ((nextChar = getc(in_fp)) != EOF) {
		//#ifdef DEBUG_TOKEN
		printf("입력받은 글자 : %c\n", nextChar);
		//#endif
		if (judge(nextChar) != 10) {
			// 맨 처음 입력인가?
			if (prev_flag == 0) {
				// 저 nextChar가 뭔지 판단해주는 함수
				now_flag = judge(nextChar);
				prev_flag = judge(nextChar);

				buf_temp[0] = nextChar;
				printf("buf_temp[0] = %c\n", nextChar);
				buf_temp[1] = '\0';
				buf_count = 1;
#ifdef DEBUG_TOKEN
				printf("맨 처음 입력 시작!\n");
				printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
				printf("현재 저장된 token : %s\n", buf_temp);
#endif
			}
			else {
				//	printf("맨 처음 입력이 아님!\n");
				now_flag = judge(nextChar);
#ifdef DEBUG_TOKEN
				printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
#endif
				// 만약 기존에 받는 형태가 올 경우 (문자-문자, 숫자-숫자, 연산자-연산자)
				// 단, 문자-숫자는 허용
				// 단, 괄호의 경우 무조건 분리 원칙에 따름
				// ex) aa, 12, +=, a3, ...
				if (((prev_flag == now_flag) && (prev_flag != 15 && prev_flag != 16 && prev_flag != 17 && prev_flag != 18)) || (prev_flag == 12 && now_flag == 11) || (prev_flag == 13 && now_flag == 19)) {
#ifdef DEBUG_TOKEN
					printf("기존에 받는 형태 맞음!\n");
#endif
					buf_temp[buf_count] = nextChar;
					buf_temp[buf_count + 1] = '\0';
					buf_count++;
#ifdef DEBUG_TOKEN
					printf("현재 저장된 token : %s\n", buf_temp);
#endif
				}
				else {
					// 만약 기존에 받는 형태가 아닌 다른 형태가 올 경우 ex) 1 + , char *, - 3 ... )
					// 기존에 받았던 문자들은 따로 저장해 놓음 (buf_token 배열에 저장)
#ifdef DEBUG_TOKEN
					printf("기존에 받는 형태가 아니다!\n");
#endif
					buf_temp[buf_count] = '\0';
					strcpy(buf_token[token_count_student], buf_temp);
#ifdef DEBUG_TOKEN
					printf("최종 저장되는 token : %s\n", buf_token[token_count_student]);
#endif
					token_count_student++;

					// 새로운 토큰 저장 시작 (buf_temp에 다시 받기 시작)
					prev_flag = judge(nextChar);
#ifdef DEBUG_TOKEN
					printf("\n\n새로운 토큰 시작\n");
					printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
#endif
					buf_temp[0] = nextChar;
					buf_temp[1] = '\0';
#ifdef DEBUG_TOKEN
					printf("현재 저장된 token : %s\n", buf_temp);
#endif
					buf_count = 1;
				}
			}
		}
		else {
			printf("공백을 받았네!\n");
			sleep(2);
		}

	}

	fclose(in_fp);

	// EOF를 만나서 빠져나왔을 경우, 제일 마지막에 존재했던 토큰을 buf_token 배열에 저장해 주자
	buf_temp[buf_count] = '\0';
	strcpy(buf_token[token_count_student], buf_temp);
	token_count_student++;

	// 출력해 보기
	int i_token = 0;
	printf("학생파일!!\n");
	for (i_token = 0; i_token < token_count_student; i_token++) {
		printf("token : %s\n", buf_token[i_token]);
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////
	//                                 정답 토큰분리                                               //
	/////////////////////////////////////////////////////////////////////////////////////////////////

	char trueset_answer[1024];
	FILE *fp_answer;
	char answer_path[1024];
	strcpy(answer_path, answer_txt_path);

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

	// 문제의 정답 파일을 토큰 분리한다.
	// 문제의 정답 파일은 char *trueset_answer에 있음
	char *token_q;
	char *ptr_q[2];
	char tmp_q[100][100];
	char tmp2_q[100][100];
	token_q = strtok_r(trueset_answer, ":", &ptr_q[0]);

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

	int space_problem_token = i_q;
	int problem_token = j_q;
#ifdef DEBUG
	printf("문제토큰 i는 %d, j는 %d\n", i_q, j_q);
#endif
	int tmp_size = strlen(tmp2_q[0]);
	if (problem_token != 1 && problem_token > 0) {
		tmp2_q[0][tmp_size - 1] = '\0';
	}

	int k_q = 0;
	for (i_q = 1; i_q < problem_token; i_q++) {
		tmp_size = strlen(tmp2_q[i_q]);
		for (k_q = 0; k_q < tmp_size - 1; k_q++) {
			tmp2_q[i_q][k_q] = tmp2_q[i_q][k_q + 1];
		}
		if (i_q == problem_token - 1) {
			tmp2_q[i_q][tmp_size - 1] = '\0';
		}
		else {
			tmp2_q[i_q][tmp_size - 2] = '\0';
		}
	}

#ifdef DEBUG
	for (i_q = 0; i_q < 10; i_q++)
		printf("tmp_q[%d]: %s\n", i_q, tmp_q[i_q]);

	for (i_q = 0; i_q < 10; i_q++)
		printf("tmp2_q[%d]: %s\n", i_q, tmp2_q[i_q]);
#endif

	// tmp2_q가 의미있는 값임!
	//	exit(0);

	// 한 글자씩 입력받자.
	int token_count = 0;
	buf_count = 0;
	char buf_temp2[500];
	char buf_token2[500][500];

	prev_flag = 0;
	now_flag = 0;

	int i_p_count = 0;
	int k_count = 0;

	for (k_count = 0; k_count < j_q; k_count++) {
		i_p_count = 0;
		prev_flag = 0;
		token_count = 0;
		buf_count = 0;

		printf("\n\n%d번째 답에 대한 판정을 시작!\n\n\n", k_count + 1);


		while ((nextChar = tmp2_q[k_count][i_p_count]) != '\0') {
#ifdef DEBUG_TOKEN
			printf("입력받은 글자 : %c\n", nextChar);
#endif

			i_p_count++;
			if (judge(nextChar) != 10) {
				// 맨 처음 입력인가?
				if (prev_flag == 0) {
					// 저 nextChar가 뭔지 판단해주는 함수
					now_flag = judge(nextChar);
					prev_flag = judge(nextChar);
					buf_temp2[0] = nextChar;
					buf_temp2[1] = '\0';
					buf_count = 1;
#ifdef DEBUG_TOKEN
					printf("맨 처음 입력 시작!\n");
					printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
					printf("현재 저장된 token : %s\n", buf_temp2);
#endif
				}
				else {
					//	printf("맨 처음 입력이 아님!\n");
					now_flag = judge(nextChar);
#ifdef DEBUG_TOKEN
					printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
#endif
					// 만약 기존에 받는 형태가 올 경우 (문자-문자, 숫자-숫자, 연산자-연산자)
					// 단, 문자-숫자는 허용
					// 단, 괄호의 경우 무조건 분리 원칙에 따름
					// ex) aa, 12, +=, a3, ...
					if (((prev_flag == now_flag) && (prev_flag != 15 && prev_flag != 16 && prev_flag != 17 && prev_flag != 18)) || (prev_flag == 12 && now_flag == 11) || (prev_flag == 13 && now_flag == 19)) {
#ifdef DEBUG_TOKEN
						printf("기존에 받는 형태 맞음!\n");
#endif
						buf_temp2[buf_count] = nextChar;
						buf_temp2[buf_count + 1] = '\0';
						buf_count++;
#ifdef DEBUG_TOKEN
						printf("현재 저장된 token : %s\n", buf_temp2);
#endif
					}
					else {
						// 만약 기존에 받는 형태가 아닌 다른 형태가 올 경우 ex) 1 + , char *, - 3 ... )
						// 기존에 받았던 문자들은 따로 저장해 놓음 (buf_token 배열에 저장)
#ifdef DEBUG_TOKEN
						printf("기존에 받는 형태가 아니다!\n");
#endif
						buf_temp2[buf_count] = '\0';
						strcpy(buf_token2[token_count], buf_temp2);
#ifdef DEBUG_TOKEN
						printf("최종 저장되는 token : %s\n", buf_token2[token_count]);
#endif
						token_count++;

						// 새로운 토큰 저장 시작 (buf_temp에 다시 받기 시작)
						prev_flag = judge(nextChar);
#ifdef DEBUG_TOKEN
						printf("\n\n새로운 토큰 시작\n");
						printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
#endif
						buf_temp2[0] = nextChar;
						buf_temp2[1] = '\0';
#ifdef DEBUG_TOKEN
						printf("현재 저장된 token : %s\n", buf_temp2);
#endif
						buf_count = 1;
					}
				}
			}
		}

		//	fclose(in_fp);

		// EOF를 만나서 빠져나왔을 경우, 제일 마지막에 존재했던 토큰을 buf_token 배열에 저장해 주자
		buf_temp2[buf_count] = '\0';
		strcpy(buf_token2[token_count], buf_temp2);
		token_count++;


		printf("정답파일!!\n");
		// 출력해 보기
		for (i_token = 0; i_token < token_count; i_token++) {
			printf("token : %s\n", buf_token2[i_token]);
		}

		////////////////////////////////////////////////////////////////////////////////////////////
		//                       학생파일과 정답파일을 비교해 보기!                               //
		////////////////////////////////////////////////////////////////////////////////////////////

		printf("둘을 비교해 보자!\n");
		int correct_sw_token = 1;
		for (i_token = 0; i_token < token_count; i_token++) {
			if (strcmp(buf_token[i_token], buf_token2[i_token]) != 0) {
				printf("%s와 %s가 달라요! %d번째!\n", buf_token[i_token], buf_token2[i_token], i_token);
				correct_sw_token = 0;
				break;
			}
		}

		sleep(3);

		///////////////////////////////////////////////////////////////////////////////////////
		//                          정답 파일 파싱을 시도해보기                              //
		///////////////////////////////////////////////////////////////////////////////////////

		// 1번 방법 따로, 2번 방법 따로, 3번 방법 따로, 4번 방법 따로 하여야 하며,
		// 순서를 역순으로 해야 함
		// 왜냐하면, 다음을 생각해 보자.
		// 정답 파일 : value < 30 || value > 29
		// 학생 파일 : 29 < value || value < 30
		// 이 경우 || 연산을 먼저 시도할 경우, value > 29와 29 < value는 같은 연산인데도 불구하고 다르게 처리된다.
		// 따라서, < 연산에 대한 파싱을 먼저 시도하여, value > 29로 바로잡고,
		// || 연산을 시도하여 value < 30 || value > 29 로 학생 토큰을 바꿔서 정답 처리하는 것이 타당하다.
		//
		// but, 연산자 우선순위 순으로 시도하다가, 만약 괄호로 덮어져 있는 블록을 만나면,
		// 그 블록 내에서 하위 연산자 우선순위를 시도하여야 한다.
		// ex ) -1 == (fd1 = open(filename, O_RDWR | O_APPEND , 0644 ) ) 에서, == 을 먼저 시도하여
		// 	-1과 (fd1 = open(filename, O_RDWR | O_APPEND , 0644 ) ) 로 될 경우,
		// 	안에 괄호가 있기 때문에 그 부분에 대해서는 하위 연산자인 ||, &&, |, &에 대한 탐색을 시도하여야 한다.
		//	따라서, 그 괄호 안에서 이루어지는 연산에 대해서도 고려해 주어야 한다.
		//	상위 연산은 이미 앞에서 고려되었을 것이므로, 다루지 않는다.


		printf("정답 파일 파싱을 시도합니다...!\n");

		// 1. 괄호를 찾아서 없애 본다.
		// 괄호가 없어졌을 때 식이 성립하면 정답!
		// 아닐 경우 2,3,4 파싱 시도!

		int a = 0, b = 0;
		if (correct_sw_token == 0) {

			char stack[500][500];
			char stack_student[500][500];
			// 정답 토큰 개수 : token_count (개)

			int i_i_token = 0;
			for (i_token = 0; i_token < token_count; i_token++) {
				printf("buf_token2[%d] = %s\n", i_token, buf_token2[i_token]);

				if ((strcmp(buf_token2[i_i_token], "(") == 0) || (strcmp(buf_token2[i_i_token], ")") == 0)) {
					printf("괄호를 찾음! 이부분은 제외\n");
				}
				else {
					strcpy(stack[i_i_token], buf_token2[i_i_token]);
					i_i_token++;
				}
			}

			// 학생 토큰 개수 : token_count_student (개)
			char stack_std[500][500];

			int i_i_std_token = 0;
			for (i_token = 0; i_token < token_count_student; i_token++) {
				printf("buf_token[%d] = %s\n", i_token, buf_token[i_token]);

				if ((strcmp(buf_token[i_i_std_token], "(") == 0) || (strcmp(buf_token[i_i_std_token], ")") == 0)) {
					printf("괄호를 찾음! 이부분은 제외\n");
				}
				else {
					strcpy(stack_student[i_i_std_token], buf_token[i_i_std_token]);
					i_i_std_token++;
				}
			}


			// 정답 토큰과 학생 토큰 비교
			int i_i_token_i_i_std_token = 0;
			int flag_true = 0;
			if (i_i_std_token == i_i_token) {
				for (i_i_token_i_i_std_token = 0; i_i_token_i_i_std_token < i_i_token; i_i_token_i_i_std_token++) {
					printf("비교중... 정답 토큰은 %s, 학생 토큰은 %s\n", stack[i_i_token_i_i_std_token], stack_student[i_i_token_i_i_std_token]);
					if (strcmp(stack[i_i_token_i_i_std_token], stack_student[i_i_token_i_i_std_token]) != 0) {
						flag_true = 1;
						break;
					}
				}
				if (flag_true == 0) {
					printf("성공! 1이 리턴됩니다!");
					return 1;
				}
			}
			else {
				// 2번 함수 비교 파싱 시작 - 밑에 3번 함수, 4번 함수도 순찰....
				a = second_parsing(correct_sw_token, i_i_token, i_i_std_token, stack, stack_std);
				b = second_parsing(correct_sw_token, token_count, token_count_student, buf_token2, buf_token);
			}
		}

		// 최종 결과를 쓰게 함
		printf("최종 결과 : ");
		printf("a: %d, b : %d\n", a, b);
		if (correct_sw_token == 0) {
			printf("오답!\n"); return 0;
		}
		else {
			printf("정답!\n");
			return 1;
		}
		// main함수 종료
	}

}


int judge(char nextChar) {

	// 만약 nextChar가 공백이면
	if (isspace(nextChar)) {
		return 10;
	}

	// 만약 nextChar가 숫자이면
	if (isdigit(nextChar)) {
		return 11;
	}

	// 만약 nextChar가 문자이면
	if (isalpha(nextChar) || nextChar == '_') {
		return 12;
	}

	// 만약 nextChar가 연산자이면
	switch (nextChar) {
	case '+':
	case '-':
	case '/':
	case '*':
	case '%':
	case '|':
	case '&':
		return 13;

		// 만약 nextChar가 comma이면
	case ',':
		return 14;

		// 만약 nextChar가 '(' 이면
	case '(':
		return 15;

		// 만약 nextChar가 ')' 이면
	case ')':
		return 16;

	case '[':
		return 17;

	case ']':
		return 18;

		// = 연산자는 조금 특별하게 보자.
		// = 뒤에 =이 와서 ==이 되지 않는 한,
		// 분리를 해 주는게 좋음
		// ==-1의 경우 ==과 - 1로 토큰이 분리되면 좋은데, 그러지 못함.
	case '<':
	case '>':
	case '=':
		return 19;

	case EOF:
		return 20;

	default:
		return 21;
	}

}