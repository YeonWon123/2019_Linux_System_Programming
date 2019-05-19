#include <stdio.h>
#include <stdlib.h>	// exit(0) ��� ����
#include <ctype.h>  // isdigit(), isalpha(), isspace() ��� ����
#include <string.h> // ���ڿ� �Լ� ��� ����

char nextChar;
FILE *in_fp;

// nextChar�� ���� �Ǵ��� �ִ� �Լ�
int judge(char nextChar);

int now_flag = 0;  // ���� ���¸� ��Ÿ��
int prev_flag = 0; // �ٷ� �� ���¸� ��Ÿ��

int main(void) {
	/* Open the input data file and process its contents */
	if ((in_fp = fopen("front.txt", "r")) == NULL) {
		printf("ERROR - cannot open front.in \n");
		exit(0);
	}
	
	// �� ���ھ� �Է¹���.
	int token_count = 0;
	int buf_count = 0;
	char buf_temp[1024];
	char buf_token[1024][1024];

	while (nextChar = getc(in_fp) != EOF) {

		// �� ó�� �Է��ΰ�?
		if (prev_flag == 0) {
			// �� nextChar�� ���� �Ǵ����ִ� �Լ�
			now_flag = judge(nextChar);
			prev_flag = judge(nextChar);
			buf_temp[0] = nextChar;
			buf_temp[1] = '\0';
			buf_count = 1;
			printf("�� ó�� �Է� ����!\n");
			printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
			printf("���� ����� token : %s\n", buf_temp);
		}
		else {
			printf("�� ó�� �Է��� �ƴ�!\n");
			now_flag = judge(nextChar);
			printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);

			// ���� ������ �޴� ���°� �� ��� (����-����, ����-����, ������-������)
			// ��, ����-���ڴ� ���
			// ex) aa, 12, +=, a3, ...
			if (prev_flag == now_flag || (prev_flag == 12 && now_flag == 11)) {
				printf("������ �޴� ���� ����!\n");
				buf_temp[buf_count] = nextChar;
				buf_temp[buf_count + 1] = '\0';
				buf_count++;
				printf("���� ����� token : %s\n", buf_temp);
			}
			else {
				// ���� ������ �޴� ���°� �ƴ� �ٸ� ���°� �� ��� ex) 1 + , char *, - 3 ... )
				// ������ �޾Ҵ� ���ڵ��� ���� ������ ���� (buf_token �迭�� ����)
				printf("������ �޴� ���°� �ƴϴ�!\n");
				buf_temp[buf_count] = '\0';
				strcpy(buf_token[token_count], buf_temp);
				printf("���� ����Ǵ� token : %s\n", buf_token[token_count]);
				token_count++;
	
				// ���ο� ��ū ���� ���� (buf_temp�� �ٽ� �ޱ� ����)
				prev_flag = judge(nextChar);
				printf("\n\n���ο� ��ū ����\n");
				printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
				buf_temp[0] = nextChar;
				buf_temp[1] = '\0';
				printf("���� ����� token : %s\n", buf_temp);
				buf_count = 1;
			}
		}

	}
	// EOF�� ������ ���������� ���, ���� �������� �����ߴ� ��ū�� buf_token �迭�� ������ ����
	buf_temp[buf_count] = '\0';
	strcpy(buf_token[token_count], buf_temp);
	token_count++;

	// ����� ����
	int i_token = 0;
	for (i_token = 0; i_token < token_count; i_token++) {
		printf("token : %s\n", buf_token[i_token]);
	}

	exit(0);	
}

int judge(char nextChar) {

	// ���� nextChar�� �����̸�
	if (isspace(nextChar)) {
		return 10;
	}

	// ���� nextChar�� �����̸�
	if (isdigit(nextChar)) {
		return 11;
	}

	// ���� nextChar�� �����̸�
	if (isdigit(nextChar) || nextChar == '_') {
		return 12;
	}

	// ���� nextChar�� �������̸�
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

	// ���� nextChar�� comma�̸�
	case ',':
		return 14;

	// ���� nextChar�� '(' �̸�
	case '(':
		return 15;

	// ���� nextChar�� ')' �̸�
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