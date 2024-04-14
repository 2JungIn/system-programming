/**
 * Reference
 * [1] https://ko.wikipedia.org/wiki/%EC%97%94%EB%94%94%EC%96%B8
 * 
 * endian.c
 * 
 * 이 컴퓨터의 바이트 순서가 어떤 엔디안을 사용하는지 출력하는 프로그램 입니다.
**/

#include <stdio.h>

int main(void)
{
    /**
     * +---------------------------------+
     * |       0x12345678 의 표현        |
     * +-------------+----+----+----+----+
     * |    인덱스   |  0 |  1 |  2 |  3 |
     * +-------------+----+----+----+----+
     * |  빅 엔디안  | 12 | 34 | 56 | 78 |
     * +-------------+----+----+----+----+
     * | 리틀 엔디안 | 78 | 65 | 43 | 21 |
     * +-------------+----+----+----+----+
    **/
    unsigned int data = 0x12345678;

    unsigned char *ptr = (unsigned char *)&data;
    if (ptr[0] == 0x12)
        printf("big endian!\n");
    else if (ptr[0] == 0x78)
        printf("little endian!\n");
    else
        printf("Unkwon!\n");

    return 0;
}