#include <stdio.h>
#include <stdlib.h>	// exit(0) 사용 가능
#include <ctype.h>  // isdigit(), isalpha(), isspace() 사용 가능
#include <string.h> // 문자열 함수 사용 가능

char nextChar;
FILE *in_fp;

// nextChar가 뭔지 판단해 주는 함수
int judge(char nextChar);

int now_flag = 0;  // 지금 상태를 나타냄
int prev_flag = 0; // 바로 전 상태를 나타냄

int main(void) {
	/* Open the input data file and process its contents */
	if ((in_fp = fopen("front.txt", "r")) == NULL) {
		printf("ERROR - cannot open front.in \n");
		exit(0);
	}
	
	// 한 글자씩 입력받자.
	int token_count = 0;
	int buf_count = 0;
	char buf_temp[1024];
	char buf_token[1024][1024];

	while (nextChar = getc(in_fp) != EOF) {

		// 맨 처음 입력인가?
		if (prev_flag == 0) {
			// 저 nextChar가 뭔지 판단해주는 함수
			now_flag = judge(nextChar);
			prev_flag = judge(nextChar);
			buf_temp[0] = nextChar;
			buf_temp[1] = '\0';
			buf_count = 1;
			printf("맨 처음 입력 시작!\n");
			printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
			printf("현재 저장된 token : %s\n", buf_temp);
		}
		else {
			printf("맨 처음 입력이 아님!\n");
			now_flag = judge(nextChar);
			printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);

			// 만약 기존에 받는 형태가 올 경우 (문자-문자, 숫자-숫자, 연산자-연산자)
			// 단, 문자-숫자는 허용
			// ex) aa, 12, +=, a3, ...
			if (prev_flag == now_flag || (prev_flag == 12 && now_flag == 11)) {
				printf("기존에 받는 형태 맞음!\n");
				buf_temp[buf_count] = nextChar;
				buf_temp[buf_count + 1] = '\0';
				buf_count++;
				printf("현재 저장된 token : %s\n", buf_temp);
			}
			else {
				// 만약 기존에 받는 형태가 아닌 다른 형태가 올 경우 ex) 1 + , char *, - 3 ... )
				// 기존에 받았던 문자들은 따로 저장해 놓음 (buf_token 배열에 저장)
				printf("기존에 받는 형태가 아니다!\n");
				buf_temp[buf_count] = '\0';
				strcpy(buf_token[token_count], buf_temp);
				printf("최종 저장되는 token : %s\n", buf_token[token_count]);
				token_count++;
	
				// 새로운 토큰 저장 시작 (buf_temp에 다시 받기 시작)
				prev_flag = judge(nextChar);
				printf("\n\n새로운 토큰 시작\n");
				printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
				buf_temp[0] = nextChar;
				buf_temp[1] = '\0';
				printf("현재 저장된 token : %s\n", buf_temp);
				buf_count = 1;
			}
		}

	}
	// EOF를 만나서 빠져나왔을 경우, 제일 마지막에 존재했던 토큰을 buf_token 배열에 저장해 주자
	buf_temp[buf_count] = '\0';
	strcpy(buf_token[token_count], buf_temp);
	token_count++;

	// 출력해 보기
	int i_token = 0;
	for (i_token = 0; i_token < token_count; i_token++) {
		printf("token : %s\n", buf_token[i_token]);
	}

	exit(0);	
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
	if (isdigit(nextChar) || nextChar == '_') {
		return 12;
	}

	// 만약 nextChar가 연산자이면
	switch (nextChar) {
	case '+':
	case '-':
	case '/':
	case '*':
	case '%':
	case '=':
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

	case EOF:
		return 20;

	default:
		return 21;
	}

}