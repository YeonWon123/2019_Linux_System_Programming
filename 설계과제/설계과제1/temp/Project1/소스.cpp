#include <stdio.h>
#include <stdlib.h>  	// exit(0) ��� ����
#include <ctype.h>      // isdigit(), isalpha(), isspace() ��� ����
#include <string.h>     // ���ڿ� �Լ� ��� ����

//#define DEBUG_TOKEN
//#define DEBUG

char nextChar;
FILE *in_fp;

// nextChar�� ���� �Ǵ��� �ִ� �Լ�
int judge(char nextChar);

// �׹�° �����ڿ� ���� �Ľ�
// 4. ||, &&, |, &�� ã�´�. ���� �����Ѵٸ�, �¿�� �������� �Ǵ��Ѵ�.
// ��, �������� �Ǵ��� ���� �� (�� ������ �� ���� �������� �����Ѵ�.
// ���������� �Ǵ��� ���� ���� )�� ������ �� ���� �������� �����Ѵ�.
// ��ȣ�� �ѷ������� ���� ��Ȳ���� ,�� ������ �����Ѵ�.

// �Ѱ� : ,a | b | c | d, �� ���� ��
// (b | c | d ) | a (����)
// (c | d) | (a | b) (����)
// d | (a | b | c ) (����)
//
// c | b | a | d (�Ұ���)
// d | c | b | a (�Ұ���)

int fourth_parsing(int correct_sw_token, int token_count, int token_count_student, char buf_token[][500], char buf_token2[][500]) {
	if (correct_sw_token == 0) {

		char left_stack[500];
		char right_stack[500];
		left_stack[0] = '\0';
		right_stack[0] = '\0';

		// ���� ��ū ���� : token_count (��)
		int i_token;
		for (i_token = 0; i_token < token_count; i_token++) {
			printf("buf_token2[%d] = %s\n", i_token, buf_token2[i_token]);

			if ((strcmp(buf_token2[i_token], "||") == 0) || (strcmp(buf_token2[i_token], "&&") == 0) ||
				(strcmp(buf_token2[i_token], "|") == 0) || (strcmp(buf_token2[i_token], "&") == 0)) {

				printf("�� 4���� �����ڸ� ã��!\n");

				// �ʱ�ȭ
				left_stack[0] = '\0';
				right_stack[0] = '\0';


				int j_token = i_token - 1;
				int left_paren = 0;
				int right_paren = 0;

				// ���� �Ǵ�
				for (j_token = i_token - 1; j_token > -1; j_token--) {

					// ���ڿ� �и� ����غ���!
					// ���� ��ȣ�ΰ�?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						// ���ڰ� �� ������ break
						// �ƴϾ count ����
						left_paren++;
						if (left_paren > right_paren) break;
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// ������ ��ȣ�ΰ�?
						right_paren++;
					}
					else if (strcmp(buf_token2[j_token], ",") == 0) {
						// ���� ��ȣ�� �ѷ������� ���� ��Ȳ���� ,�� ������ break
						if (left_paren >= right_paren) {
							j_token++;
							break;
						}
					}

					// 
					// strcat(left_stack, buf_token2[j_token]);
					// debug
					// printf("���� �Ǵ� : %s\n", left_stack);
				}

				// ���� �Ǵ��� ���, ���� strcat�� �� ��� �ݴ�� �Է°��� ����!
				// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
				// �� ���, �� �ڿ� �ִ� �ź��� �������� �ٽ� ���� ������ ����
				// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
				// ����, �տ������� �ٿ�����
				int left_token_offset_count;
				for (left_token_offset_count = j_token; left_token_offset_count <= i_token - 1; left_token_offset_count++) {
					strcat(left_stack, buf_token2[left_token_offset_count]);
				}

				// debug
				printf("���� ��ū ���� �Ǵ� : %s\n", left_stack);
				//	exit(0);

				j_token = i_token;
				left_paren = 0;
				right_paren = 0;

				// ������ �Ǵ�
				for (j_token = i_token + 1; j_token < token_count; j_token++) {

					// ���� ��ȣ�ΰ�?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						left_paren++;
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// ������ ��ȣ�ΰ�?
						// ���ڰ� �� ������ break
						// �ƴϾ count ����
						right_paren++;
						if (right_paren > left_paren) break;
					}
					else if (strcmp(buf_token2[j_token], ",") == 0) {
						// ��ȣ�� �ѷ������� ���� ��Ȳ���� ,�� ������ break
						if (right_paren >= left_paren) {
							j_token--;
							break;
						}
					}

					strcat(right_stack, buf_token2[j_token]);
					// debug
					printf("������ �Ǵ� : %s\n", right_stack);
				}




				// �̰�, �л� ���ϵ� �Ľ��ؼ� �� �κ��� �ִ��� ���� ���ƾ� ��!
				// �л� ���� ��ū �����ϴ� �迭 : buf_token[]
				// �л� ���� ��ū ���� : token_count_student


				// ����(������) ��ū ���� offset, �� offset
				int left_token_offset[2] = { 0, 0 };
				int right_token_offset[2] = { 0, 0 };


				char left_stack_stu[500];
				char right_stack_stu[500];
				left_stack_stu[0] = '\0';
				right_stack_stu[0] = '\0';

				int i_token_2 = 0;

				// �л� ���� ������ ���� �񱳸� ������
				for (i_token_2 = 0; i_token_2 < token_count_student; i_token_2++) {
					printf("buf_token[%d] = %s\n", i_token_2, buf_token[i_token_2]);
					if ((strcmp(buf_token[i_token_2], "||") == 0) || (strcmp(buf_token[i_token_2], "&&") == 0) ||
						(strcmp(buf_token[i_token_2], "|") == 0) || (strcmp(buf_token[i_token_2], "&") == 0)) {

						printf("\n\n�л� ���� �Ľ� �õ�!\n");

						printf("�� 4���� �����ڸ� ã��!\n");

						// �ʱ�ȭ
						left_stack_stu[0] = '\0';
						right_stack_stu[0] = '\0';

						int j_token_2 = i_token_2 - 1;
						//		t_count = 0;
						left_paren = 0;
						right_paren = 0;

						// ���� �Ǵ�
						for (j_token_2 = i_token_2 - 1; j_token_2 > -1; j_token_2--) {

							// ���ڿ� �и� ����غ���!
							// ���� ��ȣ�ΰ�?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								// ���ڰ� �� ������ break
								// �ƴϾ count ����
								left_paren++;
								if (left_paren > right_paren) {
									j_token_2++;
									break;
								}
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// ������ ��ȣ�ΰ�?
								right_paren++;
							}
							else if (strcmp(buf_token[j_token_2], ",") == 0 || strcmp(buf_token[j_token_2], "=") == 0) {
								// ���� ��ȣ�� �ѷ������� ���� ��Ȳ���� , �Ǵ� =�� ������ break (������ �켱������ ����)
								if (left_paren >= right_paren) {
									j_token_2++;
									break;
								}
							}

							// 
							//strcat(left_stack_stu, buf_token[j_token_2]);
							// debug
							//printf("�л� ��ū ���� �Ǵ� : %s\n", left_stack_stu);
						}

						// ���� �Ǵ��� ���, ���� strcat�� �� ��� �ݴ�� �Է°��� ����!
						// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
						// �� ���, �� �ڿ� �ִ� �ź��� �������� �ٽ� ���� ������ ����
						// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
						// ����, �տ������� �ٿ�����
						int left_token_offset_count;
						for (left_token_offset_count = j_token_2; left_token_offset_count <= i_token_2 - 1; left_token_offset_count++) {
							strcat(left_stack_stu, buf_token[left_token_offset_count]);
						}

						// debug
						printf("�л� ��ū ���� �Ǵ� : %s\n", left_stack_stu);
						//	exit(0);

						// ���� ��ū offset ��ġ��Ŵ
						left_token_offset[1] = i_token_2 - 1;
						left_token_offset[0] = j_token_2;


						j_token_2 = i_token_2;
						left_paren = 0;
						right_paren = 0;

						// ������ �Ǵ�
						for (j_token_2 = i_token_2 + 1; j_token_2 < token_count_student; j_token_2++) {

							// ���� ��ȣ�ΰ�?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								left_paren++;
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// ������ ��ȣ�ΰ�?
								right_paren++;
								if (right_paren > left_paren) {
									j_token_2--;
									break;
								}
							}
							else if (strcmp(buf_token[j_token_2], ",") == 0 || strcmp(buf_token[j_token_2], "=") == 0) {
								// ���� ��ȣ�� �ѷ������� ���� ��Ȳ���� , �Ǵ� =�� ������ break (������ �켱������ ����)
								if (right_paren >= left_paren) {
									j_token_2--;
									break;
								}
							}

							strcat(right_stack_stu, buf_token[j_token_2]);
							// debug
							printf("j_token_2 = %d, token_count_student = %d\n", j_token_2, token_count_student);
							printf("������ �Ǵ� : %s\n", right_stack_stu);
						}


						// ������ ��ū offset ��ġ��Ŵ
						right_token_offset[0] = i_token_2 + 1;
						right_token_offset[1] = j_token_2;


						// ���� ���ϰ� �л� ���� ����, ���� ������ ��ū�� �̵���Ŵ
						// 	
						// 	���� ��ū ���� : left_stack
						// 	���� ��ū ������ : right_stack
						// 	�л� ��ū ���� : left_stack_stu
						// 	�л� ��ū ������ : right_stack_stu
						//
						// ���� ��ū ���� == �л� ��ū ������?
						// ���� ��ū ������ == �л� ��ū ����?
						// �� ���� ������, �л� ��ū �� �κ��� �ű�� �ٽ� ��ū���� ����
						// �����ϸ� true�� ���� ������ ��������, ���н� continue�� �ݺ�

						char token_temp_part[100][500];

						printf("���� ��ū ���� : %s, ���� ��ū ������ : %s\n", left_stack, right_stack);
						printf("�л� ��ū ���� : %s, �л� ��ū ������ : %s\n", left_stack_stu, right_stack_stu);

						printf("�л� ��ū�� ���� - ����: (left_token_offset[0]) ~ (left_token_offset[1]) : %d ~ %d\n", left_token_offset[0], left_token_offset[1]);

						printf("�л� ��ū�� ���� - ������: (right_token_offset[0]) ~ (right_token_offset[1]) : %d ~ %d\n", right_token_offset[0], right_token_offset[1]);


						if (strcmp(left_stack, right_stack_stu) == 0 && strcmp(right_stack, left_stack_stu) == 0) {

							printf("���� �������� %s ���̿� �ִ� ���� �ٲ� ���ô�!\n", buf_token[left_token_offset[1] + 1]);

							// �ϴ� right_token_offset[0] �κк��� right_token_offset[1] �κ��� �л� ��ū(buf_token[])�� ������
							int i_change_token = 0;
							int i_zero_start = 0;
							for (i_change_token = right_token_offset[0]; i_change_token <= right_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}
							// ||�� | �Ǵ� &&������ �����ڸ� ����
							strcpy(token_temp_part[i_zero_start], buf_token[left_token_offset[1] + 1]);

							// left_token_offset[0] �κк��� left_token_offset[1] �κ��� �л� ��ū(buf_token[])�� ������
							i_change_token = 0;
							i_zero_start++;
							for (i_change_token = left_token_offset[0]; i_change_token <= left_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}

							int i_zero_start2 = 0;
							// DEBUG
							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								printf("\n�ٲ� ��ū ����� ����!\n\n");
								printf("token_temp_part[%d] = %s\n", i_zero_start2, token_temp_part[i_zero_start2]);
							}

							// token_temp_part�� �ִ� ��ū���� �л� ��ū ��ġ�� �ű�
							i_zero_start2 = 0;
							i_change_token = left_token_offset[0];
							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								strcpy(buf_token[i_change_token], token_temp_part[i_zero_start2]);
								i_change_token++;
							}

							// DEBUG
							printf("\n\n�ٲ� �л� ��ū�� ����� ���ô�!\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count_student; i_zero_start2++) {
								printf("buf_token[%d] = %s\n", i_zero_start2, buf_token[i_zero_start2]);
							}

							// DEBUG
							printf("\n\n���� ��ū�� �ٽ� ����� �����?\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								printf("buf_token2[%d] = %s\n", i_zero_start2, buf_token2[i_zero_start2]);
							}

							// �л� ��ū�� ���� ��ū�� ��� ���ٸ� �����Դϴ� ���
							// �׷��� ���� ��� 4.���� �׷��� ���� ���
							int one_cond_flag = 1; // ù ��° ������ �����Ǿ��� ��� 1
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								if (strcmp(buf_token[i_zero_start2], buf_token2[i_zero_start2]) != 0) {
									one_cond_flag = 0;
									printf("4��° ������ �� �׷��� ����!\n");
									break;
								}
							}

							if (one_cond_flag == 1) {
								printf("�����Դϴ�!\n");
								return 1;
							}

							//	exit(1);
						}

						// �����ϸ� �׳� ������ �� ������ (���� ���� �� ����)
						// �����ϸ�(same���� �߸�) �ؿ� �� �� ��������, ���� ó����


					}
				}
			}

			printf("4���� ��������?\n");
			printf("Ʋ��!\n");
		}
	}
	else {

		printf("����!\n");
		return 1;
	}
	printf("\n\n�� ���� ���Ͽ� ���� ������ ����!!!\n\n\n");
	printf("���� ����!\n");
	return 0;
}

// 3. ==, !=�� ã�´�. ���� �����Ѵٸ�, �¿�� �������� �Ǵ��Ѵ�.
// ��, �������� �Ǵ��� ���� �� (�� ������ �� ���� �������� �����Ѵ�.
// ���������� �Ǵ��� ���� ���� )�� ������ �� ���� �������� �����Ѵ�.
// ��ȣ�� �ѷ������� ���� ��Ȳ���� , �� ������ �����Ѵ�.
// ��ȣ �ȿ� ���� ���� ��Ȳ���� 4�� ������(||, &&, |, &)���� ������ �����Ѵ�.

int third_parsing(int correct_sw_token, int token_count, int token_count_student, char buf_token[][500], char buf_token2[][500])
{

	if (correct_sw_token == 0) {

		char left_stack_db[500][500];	// left_stack�� ��ū ����
		char right_stack_db[500][500];	// right_stack�� ��ū ����

		char left_stack_db_count = 0;	// left_stack�� ��ū ����
		char right_stack_db_count = 0;	// right_stack�� ��ū ����

		char left_stack[500];
		char right_stack[500];
		left_stack[0] = '\0';
		right_stack[0] = '\0';

		// ���� ��ū ���� : token_count (��)
		int i_token;
		for (i_token = 0; i_token < token_count; i_token++) {
			printf("buf_token2[%d] = %s\n", i_token, buf_token2[i_token]);

			if ((strcmp(buf_token2[i_token], "==") == 0) || (strcmp(buf_token2[i_token], "!=") == 0)) {

				printf("�� 3���� ������ == �Ǵ� != �� ã��!\n");

				// �ʱ�ȭ
				left_stack[0] = '\0';
				right_stack[0] = '\0';


				int j_token = i_token - 1;
				int left_paren = 0;
				int right_paren = 0;

				// ���� �Ǵ�
				for (j_token = i_token - 1; j_token > -1; j_token--) {

					// ���ڿ� �и� ����غ���!
					// ���� ��ȣ�ΰ�?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						// ���ڰ� �� ������ break
						// �ƴϾ count ����
						left_paren++;
						if (left_paren > right_paren) {
							j_token++;
							break;
						}
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// ������ ��ȣ�ΰ�?
						right_paren++;
					}
					else if ((strcmp(buf_token2[j_token], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], "=") == 0)) {
						// ���� ��ȣ�� �ѷ������� ���� ��Ȳ���� , �Ǵ� 4�� ������(||, &&, |, &)�� ������ break
						if (left_paren >= right_paren) {
							j_token++;
							break;
						}
					}

					// 
					//	strcat(left_stack, buf_token2[j_token]);
					// debug
					//	printf("���� �Ǵ� : %s\n", left_stack);
				}

				// ���� �Ǵ��� ���, ���� strcat�� �� ��� �ݴ�� �Է°��� ����!
				// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
				// �� ���, �� �ڿ� �ִ� �ź��� �������� �ٽ� ���� ������ ����
				// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
				// ����, �տ������� �ٿ�����
				int left_token_offset_count;
				left_stack_db_count = 0;
				for (left_token_offset_count = j_token; left_token_offset_count <= i_token - 1; left_token_offset_count++) {
					strcat(left_stack, buf_token2[left_token_offset_count]);
					strcpy(left_stack_db[left_token_offset_count - j_token], buf_token2[left_token_offset_count]);
					left_stack_db_count++;
				}

				// debug
				printf("���� ��ū ���� �Ǵ� : %s\n", left_stack);
				//	exit(0);



				j_token = i_token;
				left_paren = 0;
				right_paren = 0;

				// ������ �Ǵ�
				for (j_token = i_token + 1; j_token < token_count; j_token++) {

					// ���� ��ȣ�ΰ�?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						left_paren++;
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// ������ ��ȣ�ΰ�?
						// ���ڰ� �� ������ break
						// �ƴϾ count ����
						right_paren++;
						if (right_paren > left_paren) {
							j_token--;
							break;
						}
					}
					else if ((strcmp(buf_token2[j_token], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], "=") == 0)) {
						// ���� ��ȣ�� �ѷ������� ���� ��Ȳ���� , �Ǵ� 4�� ������(||, &&, |, &) ���� = �� ������ break
						if (right_paren >= left_paren) {
							j_token--;
							break;
						}
					}

					strcat(right_stack, buf_token2[j_token]);
					strcpy(right_stack_db[right_stack_db_count], buf_token2[j_token]);
					right_stack_db_count++;
					// debug
					printf("������ �Ǵ� : %s\n", right_stack);

				}




				// �̰�, �л� ���ϵ� �Ľ��ؼ� �� �κ��� �ִ��� ���� ���ƾ� ��!
				// �л� ���� ��ū �����ϴ� �迭 : buf_token[]
				// �л� ���� ��ū ���� : token_count_student


				// ����(������) ��ū ���� offset, �� offset
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

				// �л� ���� ������ ���� �񱳸� ������
				for (i_token_2 = 0; i_token_2 < token_count_student; i_token_2++) {
					printf("buf_token[%d] = %s\n", i_token_2, buf_token[i_token_2]);
					if ((strcmp(buf_token[i_token_2], "==") == 0) || (strcmp(buf_token[i_token_2], "!=") == 0)) {

						printf("\n\n�л� ���� �Ľ� �õ�!\n");

						printf("�� 3���� �����ڸ� ã��!\n");

						// �ʱ�ȭ
						left_stack_stu[0] = '\0';
						right_stack_stu[0] = '\0';

						int j_token_2 = i_token_2 - 1;
						left_paren = 0;
						right_paren = 0;

						// ���� �Ǵ�
						for (j_token_2 = i_token_2 - 1; j_token_2 > -1; j_token_2--) {

							// ���ڿ� �и� ����غ���!
							// ���� ��ȣ�ΰ�?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								// ���ڰ� �� ������ break
								// �ƴϾ count ����
								left_paren++;
								if (left_paren > right_paren) {
									j_token_2++;
									break;
								}
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// ������ ��ȣ�ΰ�?
								right_paren++;
							}
							else if ((strcmp(buf_token2[j_token], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], ",") == 0)) {
								// ���� ��ȣ�� �ѷ������� ���� ��Ȳ����, �Ǵ� 4�� ������(||, &&, |, &) �Ǵ� = �� ������ break
								if (left_paren >= right_paren) {
									j_token_2++;
									break;
								}
							}

							// 
							//strcat(left_stack_stu, buf_token[j_token_2]);
							// debug
							//printf("�л� ��ū ���� �Ǵ� : %s\n", left_stack_stu);


						}


						// ���� �Ǵ��� ���, ���� strcat�� �� ��� �ݴ�� �Է°��� ����!
						// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
						// �� ���, �� �ڿ� �ִ� �ź��� �������� �ٽ� ���� ������ ����
						// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
						// ����, �տ������� �ٿ�����
						left_stack_stu_db_count = 0;
						int left_token_offset_count;
						for (left_token_offset_count = j_token_2; left_token_offset_count <= i_token_2 - 1; left_token_offset_count++) {
							strcat(left_stack_stu, buf_token[left_token_offset_count]);
							strcpy(left_stack_stu_db[left_token_offset_count - j_token_2], buf_token[left_token_offset_count]);
							left_stack_stu_db_count++;
						}

						// debug
						printf("�л� ��ū ���� �Ǵ� : %s\n", left_stack_stu);
						//	exit(0);

						// ���� ��ū offset ��ġ��Ŵ
						left_token_offset[1] = i_token_2 - 1;
						left_token_offset[0] = j_token_2;


						j_token_2 = i_token_2;
						left_paren = 0;
						right_paren = 0;

						// ������ �Ǵ�
						for (j_token_2 = i_token_2 + 1; j_token_2 < token_count_student; j_token_2++) {

							// ���� ��ȣ�ΰ�?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								left_paren++;
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// ������ ��ȣ�ΰ�?
								right_paren++;
								if (right_paren > left_paren) {
									j_token_2--;
									break;
								}
							}
							else if ((strcmp(buf_token[j_token_2], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], "=") == 0)) {
								// ��ȣ�� �ѷ������� ���� ��Ȳ���� , �Ǵ� 4�� ������(||, &&, |, &) �Ǵ� = �� ������ break

								if (right_paren >= left_paren) {
									j_token_2--;
									break;
								}
							}

							strcat(right_stack_stu, buf_token[j_token_2]);
							strcpy(right_stack_stu_db[right_stack_db_count], buf_token[j_token_2]);
							right_stack_db_count++;
							// debug
							printf("������ �Ǵ� : %s\n", right_stack_stu);
						}


						// ������ ��ū offset ��ġ��Ŵ
						right_token_offset[0] = i_token_2 + 1;
						right_token_offset[1] = j_token_2;


						// ���� ���ϰ� �л� ���� ����, ���� ������ ��ū�� �̵���Ŵ
						// 	
						// 	���� ��ū ���� : left_stack
						// 	���� ��ū ������ : right_stack
						// 	�л� ��ū ���� : left_stack_stu
						// 	�л� ��ū ������ : right_stack_stu
						//
						// ���� ��ū ���� == �л� ��ū ������?
						// ���� ��ū ������ == �л� ��ū ����?
						// �� ���� ������, �л� ��ū �� �κ��� �ű�� �ٽ� ��ū���� ����
						// �����ϸ� true�� ���� ������ ��������, ���н� continue�� �ݺ�

						char token_temp_part[100][500];

						printf("3���� ������ ����, �߰ߵ� 3���� �����ڴ� %s\n", buf_token[left_token_offset[1] + 1]);
						printf("���� ��ū ���� : %s, ���� ��ū ������ : %s\n", left_stack, right_stack);
						printf("�л� ��ū ���� : %s, �л� ��ū ������ : %s\n", left_stack_stu, right_stack_stu);

						printf("�л� ��ū�� ���� - ����: (left_token_offset[0]) ~ (left_token_offset[1]) : %d ~ %d\n", left_token_offset[0], left_token_offset[1]);

						printf("�л� ��ū�� ���� - ������: (right_token_offset[0]) ~ (right_token_offset[1]) : %d ~ %d\n", right_token_offset[0], right_token_offset[1]);


						// ���� �� ��ū�� ���� ���� ���, ���� ������ (&& || & |)�� ���� �Ľ��� �õ��� ����.
						if (strcmp(left_stack, right_stack_stu) != 0) {
							// ���ʸ� �Ľ��� �� �´��� �����ϸ� �� ��!
							// 4��° �Լ� �ҷ�����
							fourth_parsing(0, left_stack_db_count, right_stack_stu_db_count, left_stack_db, right_stack_stu_db);
						}


						// ���� �� ��ū�� ���� ���� ���, ���� �����ڿ� ���� �Ľ��� �õ��� ����.
						if (strcmp(right_stack, left_stack_stu) != 0) {
							// ���ʸ� �Ľ��� �� �´��� �����ϸ� �� ��!
							// 4��° �Լ� �ҷ�����
							fourth_parsing(0, left_stack_stu_db_count, right_stack_db_count, right_stack_db, left_stack_stu_db);
						}

						if (strcmp(left_stack, right_stack_stu) == 0 && strcmp(right_stack, left_stack_stu) == 0) {

							printf("���� �������� %s ���̿� �ִ� ���� �ٲ� ���ô�!\n", buf_token[left_token_offset[1] + 1]);

							// �ϴ� right_token_offset[0] �κк��� right_token_offset[1] �κ��� �л� ��ū(buf_token[])�� ������
							int i_change_token = 0;
							int i_zero_start = 0;
							for (i_change_token = right_token_offset[0]; i_change_token <= right_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}
							// ==�� != ������ �����ڸ� ����
							strcpy(token_temp_part[i_zero_start], buf_token[left_token_offset[1] + 1]);

							// left_token_offset[0] �κк��� left_token_offset[1] �κ��� �л� ��ū(buf_token[])�� ������
							i_change_token = 0;
							i_zero_start++;
							for (i_change_token = left_token_offset[0]; i_change_token <= left_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}

							int i_zero_start2 = 0;
							// DEBUG

							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								printf("\n�ٲ� ��ū ����� ����!\n\n");
								printf("token_temp_part[%d] = %s\n", i_zero_start2, token_temp_part[i_zero_start2]);
							}

							// token_temp_part�� �ִ� ��ū���� �л� ��ū ��ġ�� �ű�
							i_zero_start2 = 0;
							i_change_token = left_token_offset[0];
							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								strcpy(buf_token[i_change_token], token_temp_part[i_zero_start2]);
								i_change_token++;
							}

							// DEBUG
							printf("\n\n�ٲ� �л� ��ū�� ����� ���ô�!\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count_student; i_zero_start2++) {
								printf("buf_token[%d] = %s\n", i_zero_start2, buf_token[i_zero_start2]);
							}

							// DEBUG
							printf("\n\n���� ��ū�� �ٽ� ����� �����?\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								printf("buf_token2[%d] = %s\n", i_zero_start2, buf_token2[i_zero_start2]);
							}

							// �л� ��ū�� ���� ��ū�� ��� ���ٸ� �����Դϴ� ���
							// �׷��� ���� ��� 1.���� �׷��� ���� ���
							int one_cond_flag = 1; // ù ��° ������ �����Ǿ��� ��� 1
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								if (strcmp(buf_token[i_zero_start2], buf_token2[i_zero_start2]) != 0) {
									one_cond_flag = 0;
									printf("1. ���� �׷��� ����!\n");
									break;
								}
							}

							if (one_cond_flag == 1) {
								printf("�����Դϴ�!\n");
								return 1;
							}

							//	exit(1);
						}

						// �����ϸ� �׳� ������ �� ������ (���� ���� �� ����)
						// �����ϸ�(same���� �߸�) �ؿ� �� �� ��������, ���� ó����

					}

				}

				printf("���� ���� �Ľ��� �Ϸ��Ͽ����ϴ�!\n");

			}

			printf("3���� ��������?\n");
			printf("Ʋ��!\n");
		}
	}
	else {
		printf("����!\n");
		return 1;
	}

	printf("\n\n�ϳ� ������ ����!!!\n\n\n");
	// debug
	// exit(1);

	//	return;
	printf("����° �� ����!!\n\n");
	return fourth_parsing(correct_sw_token, token_count, token_count_student, buf_token, buf_token2);
}



// 2. <, >, <=, >=�� ã�´�. ���� �����Ѵٸ�, �¿�� �������� �Ǵ��Ѵ�.
// ��, �������� �Ǵ��� ���� �� (�� ������ �� ���� �������� �����Ѵ�.
// ���������� �Ǵ��� ���� ���� )�� ������ �� ���� �������� �����Ѵ�.
// ��ȣ�� �ѷ������� ���� ��Ȳ���� ,�� ������ �����Ѵ�.
// ��ȣ �ȿ� ���� ���� ��Ȳ���� 3��, 4�� �����ڵ��� ������ �����Ѵ�.
int second_parsing(int correct_sw_token, int token_count, int token_count_student, char buf_token[][500], char buf_token2[][500]) {

	if (correct_sw_token == 0) {

		char left_stack[500];
		char right_stack[500];
		left_stack[0] = '\0';
		right_stack[0] = '\0';

		char left_stack_db[500][500];	// left_stack�� ��ū ����
		char right_stack_db[500][500];	// right_stack�� ��ū ����

		int left_stack_db_count = 0;	// left_stack�� ��ū ����
		int right_stack_db_count = 0;	// right_stack�� ��ū ����
		// ���� ��ū ���� : token_count (��)

		int i_token;
		for (i_token = 0; i_token < token_count; i_token++) {
			printf("buf_token2[%d] = %s\n", i_token, buf_token2[i_token]);

			if ((strcmp(buf_token2[i_token], "<") == 0) || (strcmp(buf_token2[i_token], ">") == 0)) {

				printf("�� 2���� ������ < �Ǵ� > �� ã��!\n");

				// �ʱ�ȭ
				left_stack[0] = '\0';
				right_stack[0] = '\0';


				int j_token = i_token - 1;
				int left_paren = 0;
				int right_paren = 0;

				// ���� �Ǵ�
				for (j_token = i_token - 1; j_token > -1; j_token--) {

					// ���ڿ� �и� ����غ���!
					// ���� ��ȣ�ΰ�?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						// ���ڰ� �� ������ break
						// �ƴϾ count ����
						left_paren++;
						if (left_paren > right_paren) {
							j_token++;
							break;
						}
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// ������ ��ȣ�ΰ�?
						right_paren++;
					}
					else if ((strcmp(buf_token2[j_token], ",") == 0) || ((strcmp(buf_token2[j_token], "==") == 0) || (strcmp(buf_token2[j_token], "!=") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], "=") == 0))) {
						// ���� ��ȣ�� �ѷ������� ���� ��Ȳ���� , �Ǵ� 3�� ������(==, !=) �Ǵ� 4�� ������(||, &&, |, &)�� ������ break
						if (left_paren >= right_paren) {
							j_token++;
							break;
						}
					}

					// 
					//	strcat(left_stack, buf_token2[j_token]);
					// debug
					//	printf("���� �Ǵ� : %s\n", left_stack);

				}


				// ���� �Ǵ��� ���, ���� strcat�� �� ��� �ݴ�� �Է°��� ����!
				// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
				// �� ���, �� �ڿ� �ִ� �ź��� �������� �ٽ� ���� ������ ����
				// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
				// ����, �տ������� �ٿ�����
				left_stack_db_count = 0;
				int left_token_offset_count;
				for (left_token_offset_count = j_token; left_token_offset_count <= i_token - 1; left_token_offset_count++) {
					strcat(left_stack, buf_token2[left_token_offset_count]);
					strcpy(left_stack_db[left_token_offset_count - j_token], buf_token2[left_token_offset_count]);
					left_stack_db_count++;
				}

				// debug
				printf("���� ��ū ���� �Ǵ� : %s\n", left_stack);
				//	exit(0);



				j_token = i_token;
				left_paren = 0;
				right_paren = 0;
				right_stack_db_count++;
				// ������ �Ǵ�
				for (j_token = i_token + 1; j_token < token_count; j_token++) {
					printf("token_count : %d, j_token : %d\n", token_count, j_token);
					// ���� ��ȣ�ΰ�?
					if (strcmp(buf_token2[j_token], "(") == 0) {
						left_paren++;
					}
					else if (strcmp(buf_token2[j_token], ")") == 0) {
						// ������ ��ȣ�ΰ�?
						// ���ڰ� �� ������ break
						// �ƴϾ count ����
						right_paren++;
						if (right_paren > left_paren) {
							j_token--;
							break;
						}
					}
					else if ((strcmp(buf_token2[j_token], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || ((strcmp(buf_token2[j_token], "==") == 0) || ((strcmp(buf_token2[j_token], "!=") == 0) || (strcmp(buf_token2[j_token], "=") == 0)))) 
					{
						// ���� ��ȣ�� �ѷ������� ���� ��Ȳ���� , �Ǵ� 3�� ������(==, !=) �Ǵ� 4�� ������(||, &&, |, &) ���� = �� ������ break
						if (right_paren >= left_paren) {
							j_token--;
							break;
						}
					}

					strcat(right_stack, buf_token2[j_token]);
					strcpy(right_stack_db[right_stack_db_count], buf_token2[j_token]);
					right_stack_db_count++;
					// debug
					printf("������ �Ǵ� : %s\n", right_stack);
				}


			// �̰�, �л� ���ϵ� �Ľ��ؼ� �� �κ��� �ִ��� ���� ���ƾ� ��!
			// �л� ���� ��ū �����ϴ� �迭 : buf_token[]
			// �л� ���� ��ū ���� : token_count_student


			// ����(������) ��ū ���� offset, �� offset
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

			// �л� ���� ������ ���� �񱳸� ������
			for (i_token_2 = 0; i_token_2 < token_count_student; i_token_2++) {
				printf("buf_token[%d] = %s\n", i_token_2, buf_token[i_token_2]);
				if ((strcmp(buf_token[i_token_2], "==") == 0) || (strcmp(buf_token[i_token_2], "!=") == 0)) {
						printf("\n\n�л� ���� �Ľ� �õ�!\n");
						printf("�� 3���� �����ڸ� ã��!\n");

						// �ʱ�ȭ
						left_stack_stu[0] = '\0';
						right_stack_stu[0] = '\0';

						int j_token_2 = i_token_2 - 1;
						left_paren = 0;
						right_paren = 0;

						// ���� �Ǵ�
						for (j_token_2 = i_token_2 - 1; j_token_2 > -1; j_token_2--) {

							// ���ڿ� �и� ����غ���!
							// ���� ��ȣ�ΰ�?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								// ���ڰ� �� ������ break
								// �ƴϾ count ����
								left_paren++;
								if (left_paren > right_paren) {
									j_token_2++;
									break;
								}
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// ������ ��ȣ�ΰ�?
								right_paren++;
							}
							else if ((strcmp(buf_token2[j_token], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], ",") == 0)) {
								// ���� ��ȣ�� �ѷ������� ���� ��Ȳ����, �Ǵ� 4�� ������(||, &&, |, &) �Ǵ� = �� ������ break
								if (left_paren >= right_paren) {
									j_token_2++;
									break;
								}
							}

							// 
							//strcat(left_stack_stu, buf_token[j_token_2]);
							// debug
							//printf("�л� ��ū ���� �Ǵ� : %s\n", left_stack_stu);


						}


						// ���� �Ǵ��� ���, ���� strcat�� �� ��� �ݴ�� �Է°��� ����!
						// ex: ))0644,O_APPEND|O_RDWR,filename(open=fd1(
						// �� ���, �� �ڿ� �ִ� �ź��� �������� �ٽ� ���� ������ ����
						// ex: (fd1=open(filename,O_RDWR|O_APPEND,0644))							
						// ����, �տ������� �ٿ�����
						left_stack_stu_db_count = 0;
						int left_token_offset_count;
						for (left_token_offset_count = j_token_2; left_token_offset_count <= i_token_2 - 1; left_token_offset_count++) {
							strcat(left_stack_stu, buf_token[left_token_offset_count]);
							strcpy(left_stack_stu_db[left_token_offset_count - j_token_2], buf_token[left_token_offset_count]);
							left_stack_stu_db_count++;
						}

						// debug
						printf("�л� ��ū ���� �Ǵ� : %s\n", left_stack_stu);
						//	exit(0);

						// ���� ��ū offset ��ġ��Ŵ
						left_token_offset[1] = i_token_2 - 1;
						left_token_offset[0] = j_token_2;


						j_token_2 = i_token_2;
						left_paren = 0;
						right_paren = 0;

						// ������ �Ǵ�
						for (j_token_2 = i_token_2 + 1; j_token_2 < token_count_student; j_token_2++) {

							// ���� ��ȣ�ΰ�?
							if (strcmp(buf_token[j_token_2], "(") == 0) {
								left_paren++;
							}
							else if (strcmp(buf_token[j_token_2], ")") == 0) {
								// ������ ��ȣ�ΰ�?
								right_paren++;
								if (right_paren > left_paren) {
									j_token_2--;
									break;
								}
							}
							else if ((strcmp(buf_token[j_token_2], ",") == 0) || (strcmp(buf_token2[j_token], "||") == 0) || (strcmp(buf_token2[j_token], "&&") == 0) || (strcmp(buf_token2[j_token], "|") == 0) || (strcmp(buf_token2[j_token], "&") == 0) || (strcmp(buf_token2[j_token], "=") == 0)) {
								// ��ȣ�� �ѷ������� ���� ��Ȳ���� , �Ǵ� 4�� ������(||, &&, |, &) �Ǵ� = �� ������ break

								if (right_paren >= left_paren) {
									j_token_2--;
									break;
								}
							}

							strcat(right_stack_stu, buf_token[j_token_2]);
							strcpy(right_stack_stu_db[right_stack_db_count], buf_token[j_token_2]);
							right_stack_db_count++;
							// debug
							printf("������ �Ǵ� : %s\n", right_stack_stu);
						}


						// ������ ��ū offset ��ġ��Ŵ
						right_token_offset[0] = i_token_2 + 1;
						right_token_offset[1] = j_token_2;


						// ���� ���ϰ� �л� ���� ����, ���� ������ ��ū�� �̵���Ŵ
						// 	
						// 	���� ��ū ���� : left_stack
						// 	���� ��ū ������ : right_stack
						// 	�л� ��ū ���� : left_stack_stu
						// 	�л� ��ū ������ : right_stack_stu
						//
						// ���� ��ū ���� == �л� ��ū ������?
						// ���� ��ū ������ == �л� ��ū ����?
						// �� ���� ������, �л� ��ū �� �κ��� �ű�� �ٽ� ��ū���� ����
						// �����ϸ� true�� ���� ������ ��������, ���н� continue�� �ݺ�

						char token_temp_part[100][500];

						printf("3���� ������ ����, �߰ߵ� 3���� �����ڴ� %s\n", buf_token[left_token_offset[1] + 1]);
						printf("���� ��ū ���� : %s, ���� ��ū ������ : %s\n", left_stack, right_stack);
						printf("�л� ��ū ���� : %s, �л� ��ū ������ : %s\n", left_stack_stu, right_stack_stu);

						printf("�л� ��ū�� ���� - ����: (left_token_offset[0]) ~ (left_token_offset[1]) : %d ~ %d\n", left_token_offset[0], left_token_offset[1]);

						printf("�л� ��ū�� ���� - ������: (right_token_offset[0]) ~ (right_token_offset[1]) : %d ~ %d\n", right_token_offset[0], right_token_offset[1]);

						// ���� �� ��ū�� ���� ���� ���, ���� �����ڿ� ���� �Ľ��� �õ��� ����.
						if (strcmp(left_stack, right_stack_stu) != 0) {

							// ���ʸ� �Ľ��� �� �´��� �����ϸ� �� ��!
							// 3��° �Լ� �ҷ�����
							third_parsing(0, left_stack_db_count, right_stack_stu_db_count, left_stack_db, right_stack_stu_db);

						}

						// ���� �� ��ū�� ���� ���� ���, ���� �����ڿ� ���� �Ľ��� �õ��� ����.
						if (strcmp(right_stack, left_stack_stu) != 0) {
							// ���ʸ� �Ľ��� �� �´��� �����ϸ� �� ��!

							// 3��° �Լ� �ҷ�����
							third_parsing(0, left_stack_stu_db_count, right_stack_db_count, right_stack_db, left_stack_stu_db);
						}

						if (strcmp(left_stack, right_stack_stu) == 0 && strcmp(right_stack, left_stack_stu) == 0) {

							printf("���� �������� %s ���̿� �ִ� ���� �ٲ� ���ô�!\n", buf_token[left_token_offset[1] + 1]);

							// �ϴ� right_token_offset[0] �κк��� right_token_offset[1] �κ��� �л� ��ū(buf_token[])�� ������
							int i_change_token = 0;
							int i_zero_start = 0;
							for (i_change_token = right_token_offset[0]; i_change_token <= right_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}
							// <�� >��, <=�� >=��, >�� <��, >=�� <= ������ �����ڸ� ����
							if (strcmp(buf_token[left_token_offset[1] + 1], "<") == 0)
								strcpy(token_temp_part[i_zero_start], ">");
							else if (strcmp(buf_token[left_token_offset[1] + 1], ">") == 0)
								strcpy(token_temp_part[i_zero_start], "<");
							else if (strcmp(buf_token[left_token_offset[1] + 1], ">=") == 0)
								strcpy(token_temp_part[i_zero_start], "<=");
							else if (strcmp(buf_token[left_token_offset[1] + 1], "<=") == 0)
								strcpy(token_temp_part[i_zero_start], ">=");
							else
								printf("�����ڰ� �߸� �Է¹޾������ϴ�!\n");

							// left_token_offset[0] �κк��� left_token_offset[1] �κ��� �л� ��ū(buf_token[])�� ������
							i_change_token = 0;
							i_zero_start++;
							for (i_change_token = left_token_offset[0]; i_change_token <= left_token_offset[1]; i_change_token++) {
								strcpy(token_temp_part[i_zero_start], buf_token[i_change_token]);
								i_zero_start++;
							}

							int i_zero_start2 = 0;
							// DEBUG

							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								printf("\n�ٲ� ��ū ����� ����!\n\n");
								printf("token_temp_part[%d] = %s\n", i_zero_start2, token_temp_part[i_zero_start2]);
							}

							// token_temp_part�� �ִ� ��ū���� �л� ��ū ��ġ�� �ű�
							i_zero_start2 = 0;
							i_change_token = left_token_offset[0];
							for (i_zero_start2 = 0; i_zero_start2 < i_zero_start; i_zero_start2++) {
								strcpy(buf_token[i_change_token], token_temp_part[i_zero_start2]);
								i_change_token++;
							}

							// DEBUG
							printf("\n\n�ٲ� �л� ��ū�� ����� ���ô�!\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count_student; i_zero_start2++) {
								printf("buf_token[%d] = %s\n", i_zero_start2, buf_token[i_zero_start2]);
							}

							// DEBUG
							printf("\n\n���� ��ū�� �ٽ� ����� �����?\n\n");
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								printf("buf_token2[%d] = %s\n", i_zero_start2, buf_token2[i_zero_start2]);
							}

							// �л� ��ū�� ���� ��ū�� ��� ���ٸ� �����Դϴ� ���
							// �׷��� ���� ��� 1.���� �׷��� ���� ���
							int one_cond_flag = 1; // ù ��° ������ �����Ǿ��� ��� 1
							for (i_zero_start2 = 0; i_zero_start2 < token_count; i_zero_start2++) {
								if (strcmp(buf_token[i_zero_start2], buf_token2[i_zero_start2]) != 0) {
									one_cond_flag = 0;
									printf("1. ���� �׷��� ����!\n");
									break;
								}
							}

							if (one_cond_flag == 1) {
								printf("�����Դϴ�!\n");
								return 1;
							}

							//	exit(1);
						}

						// �����ϸ� �׳� ������ �� ������ (���� ���� �� ����)
						// �����ϸ�(same���� �߸�) �ؿ� �� �� ��������, ���� ó����

					}

				}

				printf("���� ���� �Ľ��� �Ϸ��Ͽ����ϴ�!\n");

			}

			printf("3���� ��������?\n");
			printf("Ʋ��!\n");
			//return 0;
		}
	}
	else {
		printf("����!\n");
		return 1;
	}

	printf("\n\n�ϳ� ������ ����!!!\n\n\n");
	// debug
	// exit(1);
	return third_parsing(correct_sw_token, token_count, token_count_student, buf_token, buf_token2);
}


// parsing �Լ�
int parsing(char *student_txt_path, char *answer_txt_path) {

	int now_flag = 0;  // ���� ���¸� ��Ÿ��
	int prev_flag = 0; // �ٷ� �� ���¸� ��Ÿ��

	int correct_flag = 0; // �¾Ҵ��� Ȯ��

	printf("student_txt_path : %s\n", student_txt_path);
	printf("answer_txt_path : %s\n", answer_txt_path);

	/////////////////////////////////////////////////////////////////////////////////////////////
	//                         �л� �� ��ū �и�                                               //
	/////////////////////////////////////////////////////////////////////////////////////////////

	/* Open the input data file and process its contents */
	if ((in_fp = fopen(student_txt_path, "r")) == NULL) {
		printf("ERROR - cannot open front.in \n");
		exit(0);
	}

	// �� ���ھ� �Է¹���.
	int token_count_student = 0;
	int buf_count = 0;
	char buf_temp[1024];
	buf_temp[0] = '\0';
	char buf_token[500][500];

	while ((nextChar = getc(in_fp)) != EOF) {
		//#ifdef DEBUG_TOKEN
		printf("�Է¹��� ���� : %c\n", nextChar);
		//#endif
		if (judge(nextChar) != 10) {
			// �� ó�� �Է��ΰ�?
			if (prev_flag == 0) {
				// �� nextChar�� ���� �Ǵ����ִ� �Լ�
				now_flag = judge(nextChar);
				prev_flag = judge(nextChar);

				buf_temp[0] = nextChar;
				printf("buf_temp[0] = %c\n", nextChar);
				buf_temp[1] = '\0';
				buf_count = 1;
#ifdef DEBUG_TOKEN
				printf("�� ó�� �Է� ����!\n");
				printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
				printf("���� ����� token : %s\n", buf_temp);
#endif
			}
			else {
				//	printf("�� ó�� �Է��� �ƴ�!\n");
				now_flag = judge(nextChar);
#ifdef DEBUG_TOKEN
				printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
#endif
				// ���� ������ �޴� ���°� �� ��� (����-����, ����-����, ������-������)
				// ��, ����-���ڴ� ���
				// ��, ��ȣ�� ��� ������ �и� ��Ģ�� ����
				// ex) aa, 12, +=, a3, ...
				if (((prev_flag == now_flag) && (prev_flag != 15 && prev_flag != 16 && prev_flag != 17 && prev_flag != 18)) || (prev_flag == 12 && now_flag == 11) || (prev_flag == 13 && now_flag == 19)) {
#ifdef DEBUG_TOKEN
					printf("������ �޴� ���� ����!\n");
#endif
					buf_temp[buf_count] = nextChar;
					buf_temp[buf_count + 1] = '\0';
					buf_count++;
#ifdef DEBUG_TOKEN
					printf("���� ����� token : %s\n", buf_temp);
#endif
				}
				else {
					// ���� ������ �޴� ���°� �ƴ� �ٸ� ���°� �� ��� ex) 1 + , char *, - 3 ... )
					// ������ �޾Ҵ� ���ڵ��� ���� ������ ���� (buf_token �迭�� ����)
#ifdef DEBUG_TOKEN
					printf("������ �޴� ���°� �ƴϴ�!\n");
#endif
					buf_temp[buf_count] = '\0';
					strcpy(buf_token[token_count_student], buf_temp);
#ifdef DEBUG_TOKEN
					printf("���� ����Ǵ� token : %s\n", buf_token[token_count_student]);
#endif
					token_count_student++;

					// ���ο� ��ū ���� ���� (buf_temp�� �ٽ� �ޱ� ����)
					prev_flag = judge(nextChar);
#ifdef DEBUG_TOKEN
					printf("\n\n���ο� ��ū ����\n");
					printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
#endif
					buf_temp[0] = nextChar;
					buf_temp[1] = '\0';
#ifdef DEBUG_TOKEN
					printf("���� ����� token : %s\n", buf_temp);
#endif
					buf_count = 1;
				}
			}
		}
		else {
			printf("������ �޾ҳ�!\n");
			sleep(2);
		}

	}

	fclose(in_fp);

	// EOF�� ������ ���������� ���, ���� �������� �����ߴ� ��ū�� buf_token �迭�� ������ ����
	buf_temp[buf_count] = '\0';
	strcpy(buf_token[token_count_student], buf_temp);
	token_count_student++;

	// ����� ����
	int i_token = 0;
	printf("�л�����!!\n");
	for (i_token = 0; i_token < token_count_student; i_token++) {
		printf("token : %s\n", buf_token[i_token]);
	}


	/////////////////////////////////////////////////////////////////////////////////////////////////
	//                                 ���� ��ū�и�                                               //
	/////////////////////////////////////////////////////////////////////////////////////////////////

	char trueset_answer[1024];
	FILE *fp_answer;
	char answer_path[1024];
	strcpy(answer_path, answer_txt_path);

	// ���������� �ҷ��� �� ���� ���, �� ������ ���� ��
	if ((fp_answer = fopen(answer_path, "r")) == NULL) {
		fprintf(stderr, "�������� �� %s file doesn't exist! ���α׷� ����!\n", answer_path);
		exit(1);
	}
	else {
		// ���������� �ҷ��Դٸ�, trueset_answer�� �ʱ�ȭ�� ����,  trueset_answer�� ����
		trueset_answer[0] = '\0';
		fscanf(fp_answer, "%[^\n]", trueset_answer);
#ifdef DEBUG
		printf("�������� : %s\n", trueset_answer);
#endif
	}

	// ������ ���� ������ ��ū �и��Ѵ�.
	// ������ ���� ������ char *trueset_answer�� ����
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
	printf("������ū i�� %d, j�� %d\n", i_q, j_q);
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

	// tmp2_q�� �ǹ��ִ� ����!
	//	exit(0);

	// �� ���ھ� �Է¹���.
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

		printf("\n\n%d��° �信 ���� ������ ����!\n\n\n", k_count + 1);


		while ((nextChar = tmp2_q[k_count][i_p_count]) != '\0') {
#ifdef DEBUG_TOKEN
			printf("�Է¹��� ���� : %c\n", nextChar);
#endif

			i_p_count++;
			if (judge(nextChar) != 10) {
				// �� ó�� �Է��ΰ�?
				if (prev_flag == 0) {
					// �� nextChar�� ���� �Ǵ����ִ� �Լ�
					now_flag = judge(nextChar);
					prev_flag = judge(nextChar);
					buf_temp2[0] = nextChar;
					buf_temp2[1] = '\0';
					buf_count = 1;
#ifdef DEBUG_TOKEN
					printf("�� ó�� �Է� ����!\n");
					printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
					printf("���� ����� token : %s\n", buf_temp2);
#endif
				}
				else {
					//	printf("�� ó�� �Է��� �ƴ�!\n");
					now_flag = judge(nextChar);
#ifdef DEBUG_TOKEN
					printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
#endif
					// ���� ������ �޴� ���°� �� ��� (����-����, ����-����, ������-������)
					// ��, ����-���ڴ� ���
					// ��, ��ȣ�� ��� ������ �и� ��Ģ�� ����
					// ex) aa, 12, +=, a3, ...
					if (((prev_flag == now_flag) && (prev_flag != 15 && prev_flag != 16 && prev_flag != 17 && prev_flag != 18)) || (prev_flag == 12 && now_flag == 11) || (prev_flag == 13 && now_flag == 19)) {
#ifdef DEBUG_TOKEN
						printf("������ �޴� ���� ����!\n");
#endif
						buf_temp2[buf_count] = nextChar;
						buf_temp2[buf_count + 1] = '\0';
						buf_count++;
#ifdef DEBUG_TOKEN
						printf("���� ����� token : %s\n", buf_temp2);
#endif
					}
					else {
						// ���� ������ �޴� ���°� �ƴ� �ٸ� ���°� �� ��� ex) 1 + , char *, - 3 ... )
						// ������ �޾Ҵ� ���ڵ��� ���� ������ ���� (buf_token �迭�� ����)
#ifdef DEBUG_TOKEN
						printf("������ �޴� ���°� �ƴϴ�!\n");
#endif
						buf_temp2[buf_count] = '\0';
						strcpy(buf_token2[token_count], buf_temp2);
#ifdef DEBUG_TOKEN
						printf("���� ����Ǵ� token : %s\n", buf_token2[token_count]);
#endif
						token_count++;

						// ���ο� ��ū ���� ���� (buf_temp�� �ٽ� �ޱ� ����)
						prev_flag = judge(nextChar);
#ifdef DEBUG_TOKEN
						printf("\n\n���ο� ��ū ����\n");
						printf("prev_flag = %d, now_flag = %d\n", prev_flag, now_flag);
#endif
						buf_temp2[0] = nextChar;
						buf_temp2[1] = '\0';
#ifdef DEBUG_TOKEN
						printf("���� ����� token : %s\n", buf_temp2);
#endif
						buf_count = 1;
					}
				}
			}
		}

		//	fclose(in_fp);

		// EOF�� ������ ���������� ���, ���� �������� �����ߴ� ��ū�� buf_token �迭�� ������ ����
		buf_temp2[buf_count] = '\0';
		strcpy(buf_token2[token_count], buf_temp2);
		token_count++;


		printf("��������!!\n");
		// ����� ����
		for (i_token = 0; i_token < token_count; i_token++) {
			printf("token : %s\n", buf_token2[i_token]);
		}

		////////////////////////////////////////////////////////////////////////////////////////////
		//                       �л����ϰ� ���������� ���� ����!                               //
		////////////////////////////////////////////////////////////////////////////////////////////

		printf("���� ���� ����!\n");
		int correct_sw_token = 1;
		for (i_token = 0; i_token < token_count; i_token++) {
			if (strcmp(buf_token[i_token], buf_token2[i_token]) != 0) {
				printf("%s�� %s�� �޶��! %d��°!\n", buf_token[i_token], buf_token2[i_token], i_token);
				correct_sw_token = 0;
				break;
			}
		}

		sleep(3);

		///////////////////////////////////////////////////////////////////////////////////////
		//                          ���� ���� �Ľ��� �õ��غ���                              //
		///////////////////////////////////////////////////////////////////////////////////////

		// 1�� ��� ����, 2�� ��� ����, 3�� ��� ����, 4�� ��� ���� �Ͽ��� �ϸ�,
		// ������ �������� �ؾ� ��
		// �ֳ��ϸ�, ������ ������ ����.
		// ���� ���� : value < 30 || value > 29
		// �л� ���� : 29 < value || value < 30
		// �� ��� || ������ ���� �õ��� ���, value > 29�� 29 < value�� ���� �����ε��� �ұ��ϰ� �ٸ��� ó���ȴ�.
		// ����, < ���꿡 ���� �Ľ��� ���� �õ��Ͽ�, value > 29�� �ٷ����,
		// || ������ �õ��Ͽ� value < 30 || value > 29 �� �л� ��ū�� �ٲ㼭 ���� ó���ϴ� ���� Ÿ���ϴ�.
		//
		// but, ������ �켱���� ������ �õ��ϴٰ�, ���� ��ȣ�� ������ �ִ� ����� ������,
		// �� ��� ������ ���� ������ �켱������ �õ��Ͽ��� �Ѵ�.
		// ex ) -1 == (fd1 = open(filename, O_RDWR | O_APPEND , 0644 ) ) ����, == �� ���� �õ��Ͽ�
		// 	-1�� (fd1 = open(filename, O_RDWR | O_APPEND , 0644 ) ) �� �� ���,
		// 	�ȿ� ��ȣ�� �ֱ� ������ �� �κп� ���ؼ��� ���� �������� ||, &&, |, &�� ���� Ž���� �õ��Ͽ��� �Ѵ�.
		//	����, �� ��ȣ �ȿ��� �̷������ ���꿡 ���ؼ��� ����� �־�� �Ѵ�.
		//	���� ������ �̹� �տ��� ����Ǿ��� ���̹Ƿ�, �ٷ��� �ʴ´�.


		printf("���� ���� �Ľ��� �õ��մϴ�...!\n");

		// 1. ��ȣ�� ã�Ƽ� ���� ����.
		// ��ȣ�� �������� �� ���� �����ϸ� ����!
		// �ƴ� ��� 2,3,4 �Ľ� �õ�!

		int a = 0, b = 0;
		if (correct_sw_token == 0) {

			char stack[500][500];
			char stack_student[500][500];
			// ���� ��ū ���� : token_count (��)

			int i_i_token = 0;
			for (i_token = 0; i_token < token_count; i_token++) {
				printf("buf_token2[%d] = %s\n", i_token, buf_token2[i_token]);

				if ((strcmp(buf_token2[i_i_token], "(") == 0) || (strcmp(buf_token2[i_i_token], ")") == 0)) {
					printf("��ȣ�� ã��! �̺κ��� ����\n");
				}
				else {
					strcpy(stack[i_i_token], buf_token2[i_i_token]);
					i_i_token++;
				}
			}

			// �л� ��ū ���� : token_count_student (��)
			char stack_std[500][500];

			int i_i_std_token = 0;
			for (i_token = 0; i_token < token_count_student; i_token++) {
				printf("buf_token[%d] = %s\n", i_token, buf_token[i_token]);

				if ((strcmp(buf_token[i_i_std_token], "(") == 0) || (strcmp(buf_token[i_i_std_token], ")") == 0)) {
					printf("��ȣ�� ã��! �̺κ��� ����\n");
				}
				else {
					strcpy(stack_student[i_i_std_token], buf_token[i_i_std_token]);
					i_i_std_token++;
				}
			}


			// ���� ��ū�� �л� ��ū ��
			int i_i_token_i_i_std_token = 0;
			int flag_true = 0;
			if (i_i_std_token == i_i_token) {
				for (i_i_token_i_i_std_token = 0; i_i_token_i_i_std_token < i_i_token; i_i_token_i_i_std_token++) {
					printf("����... ���� ��ū�� %s, �л� ��ū�� %s\n", stack[i_i_token_i_i_std_token], stack_student[i_i_token_i_i_std_token]);
					if (strcmp(stack[i_i_token_i_i_std_token], stack_student[i_i_token_i_i_std_token]) != 0) {
						flag_true = 1;
						break;
					}
				}
				if (flag_true == 0) {
					printf("����! 1�� ���ϵ˴ϴ�!");
					return 1;
				}
			}
			else {
				// 2�� �Լ� �� �Ľ� ���� - �ؿ� 3�� �Լ�, 4�� �Լ��� ����....
				a = second_parsing(correct_sw_token, i_i_token, i_i_std_token, stack, stack_std);
				b = second_parsing(correct_sw_token, token_count, token_count_student, buf_token2, buf_token);
			}
		}

		// ���� ����� ���� ��
		printf("���� ��� : ");
		printf("a: %d, b : %d\n", a, b);
		if (correct_sw_token == 0) {
			printf("����!\n"); return 0;
		}
		else {
			printf("����!\n");
			return 1;
		}
		// main�Լ� ����
	}

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
	if (isalpha(nextChar) || nextChar == '_') {
		return 12;
	}

	// ���� nextChar�� �������̸�
	switch (nextChar) {
	case '+':
	case '-':
	case '/':
	case '*':
	case '%':
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

		// = �����ڴ� ���� Ư���ϰ� ����.
		// = �ڿ� =�� �ͼ� ==�� ���� �ʴ� ��,
		// �и��� �� �ִ°� ����
		// ==-1�� ��� ==�� - 1�� ��ū�� �и��Ǹ� ������, �׷��� ����.
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