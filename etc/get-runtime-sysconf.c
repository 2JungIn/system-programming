/**
 * reference: 
 * [1] sysconf():  https://wariua.github.io/man-pages-ko/sysconf%283%29/
 * [2] https://12bme.tistory.com/217
 * [3] https://reakwon.tistory.com/231
**/

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

void print_sysconf(int name, const char *description);

int main(int argc, char *argv[])
{   
    printf("\n * 런타임에 구성 정보 얻기\n");
    printf("-------------+-------------------------------------------------------------------------\n");
    printf("  sysconf()  |설명                                               \n");
    printf("-------------+-------------------------------------------------------------------------\n");
    print_sysconf(_SC_ARG_MAX, "exec(3) 계열 함수에 대한 인자들의 최대 길이. ");
    print_sysconf(_SC_HOST_NAME_MAX, "gethostname(2)이 반환하는 호스트명의 종료용 널 바이트 제외 최대 길이. ");
    print_sysconf(_SC_NGROUPS_MAX, "그룹 ID의 최대 개수.");
    print_sysconf(_SC_LOGIN_NAME_MAX, "로그인 이름의 종료용 널 바이트 포함 최대 길이.");
    print_sysconf(_SC_CHILD_MAX, "사용자 ID 당 동시에 실행할 수 있는 프로세스의 최대값.");
    print_sysconf(_SC_VERSION, "시스템이 지원하는 POSIX의 버전.");
    print_sysconf(_SC_SEM_VALUE_MAX, "세마포어의 최대값.");
    print_sysconf(_SC_CLK_TCK, "초당 클록 틱 개수.");
    print_sysconf(_SC_TZNAME_MAX, "타임 존 이름의 최대 길이.");
    print_sysconf(_SC_TTY_NAME_MAX, "터미널 디바이스 이름의 최대 길이.");
    print_sysconf(_SC_LINE_MAX, "표준 입력이나 파일에서 유틸리티가 읽는 입력 행의 최대 길이.");
    print_sysconf(_SC_OPEN_MAX, "프로세스가 열 수 있는 파일의 최대 개수.");
    print_sysconf(_SC_STREAM_MAX, "프로세스가 열 수 있는 파일 스트림의 최대 개수.");
    print_sysconf(_SC_PASS_MAX, "패스워드의 최대 길이.");
    print_sysconf(_SC_PAGE_SIZE, "페이지의 바이트 단위 크기.");
    print_sysconf(_SC_SIGQUEUE_MAX, "한 프로세스에 시그널 큐에 담을 수 있는 시그널의 최대 개수.");
    print_sysconf(_SC_SEM_NSEMS_MAX, "한 프로세스가 동시에 사용할 수 있는 세마포어의 최대 개수.");
    print_sysconf(_SC_MQ_OPEN_MAX, "한 프로세스가 열 수 있는 최대 메시지 큐 개수.");
    printf("-------------+-------------------------------------------------------------------------\n");

    return 0;
}

void print_sysconf(int name, const char *description)
{
    long ret_val = sysconf(name);
    
    if (errno)   /* error: sysconf() */
    {
        perror("sysconf");
    }
    else
    {
        if (ret_val == -1)
            printf(" (no limit)  | %-s\n", description);
        else
            printf(" %-12ld| %-s\n", ret_val, description);
    }
}