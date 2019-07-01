#include <stdio.h>
#include <sys/uio.h>

int main(int argc, char **argv) {
	struct iovec vec[2];
	char MSG1[10] = {0,};
	char MSG2[10] = {0,};
	int str_len;

	vec[0].iov_base = MSG1; // 저장할 배열
	vec[0].iov_len = 9; // 저장할 내용의 길이
	vec[1].iov_base = MSG2;
	vec[1].iov_len = 9;

	str_len = readv(0, vec, 2);

	printf("\n총 %d 바이트 입력\n", str_len);
	printf("첫 번째 메시지 : %s \n", MSG1);
	printf("두 번째 메시지 : %s \n", MSG2);

	return 0;
}
