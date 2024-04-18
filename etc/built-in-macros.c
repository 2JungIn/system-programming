/**
 * Reference
 * [1] https://responding.tistory.com/50
 * 
 * built-in-macros.c
 * 
 * c언어 내장 매크로들을 출력하는 예제 입니다.
**/

#include <stdio.h>

int main(void)
{
    printf("현재 소스 파일의 경로: %s\n", __FILE__);
    printf("매크로가 호출된 행 번호: %d\n", __LINE__);
    printf("현재 소스파일이 마지막으로 수정되어 컴파일된 날짜: %s\n", __DATE__);
    printf("현재 소스파일이 마지막으로 수정되어 컴파일된 시간: %s\n", __TIME__);
    printf("현재 소스파일이 마지막으로 수정되어 컴파일된 시간과 날짜: %s\n", __TIMESTAMP__);
    printf("현재 매크로가 호출된 함수 이름: %s\n", __FUNCTION__);

    return 0;
}