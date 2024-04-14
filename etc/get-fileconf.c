/**
 * reference: 
 * [1] pathconf(): https://wariua.github.io/man-pages-ko/pathconf%283%29/
 * [2] https://12bme.tistory.com/217
 * [3] https://reakwon.tistory.com/231
**/

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#define DEFAULT_PATH    "/dev/tty"

void print_pathconf(const char *path, int name, const char *description);

int main(int argc, char *argv[])
{
    const char *path = DEFAULT_PATH;
    if (argc != 2)
        fprintf(stderr, "Usage: %s <file name>\n", argv[0]);
    else
        path = argv[1];
    
    printf("\n * 파일이나 디렉토리의 구성 값 얻기 (path: %s)\n", path);
    printf("-------------+-------------------------------------------------------------------------\n");
    printf(" pathconf()  |설명                                               \n");
    printf("-------------+-------------------------------------------------------------------------\n");
    print_pathconf(path, _PC_PIPE_BUF, "FIFO나 파이프에 원자적으로 기록할 수 있는 최대 바이트 수.");
    print_pathconf(path, _PC_PATH_MAX, "path 내지 fd가 현재 작업 디렉터리일 때 상대 경로명의 최대 길이.");
    print_pathconf(path, _PC_NAME_MAX, "디렉터리 path 내지 fd 내에 프로세스가 생성할 수 있는 파일명의 최대 길이.");
    print_pathconf(path, _PC_MAX_INPUT, "입력 행의 최대 길이이며 fd 내지 path가 터미널을 가리켜야 한다.");
    print_pathconf(path, _PC_LINK_MAX, "최대 링크 개수.");
    print_pathconf(path, _PC_MAX_CANON, "형식 있는 입력 행의 최대 길이이며 fd 내지 path가 터미널을 가리켜야 한다.");
    printf("-------------+-------------------------------------------------------------------------\n");

    return 0;
}

void print_pathconf(const char *path, int name, const char *description)
{
    long ret_val = pathconf(path, name);
    
    if (ret_val < 0) /* error: pathconf() */
        perror("pathconf");
    else
        printf(" %-12ld| %-s\n", ret_val, description);
}