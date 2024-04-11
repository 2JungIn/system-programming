/**
 * Reference
 * [1] https://reakwon.tistory.com/118
 * 
 * daemon-example.c
 * 
 * 1. umask를 0으로 변경한다.
 * 2. 자식 프로세스 생성 후 부모 프로세스 종료한다.
 * 3. 세로운 세션을 만들고 리더가 된다.
 * 4. SIGHUP을 무시하도록 sigaction을 설정한다.
 * 5. 작업 디렉터리를 루트 디렉토리로 변경한다.
 * 6. 열려있는 파일 디스크립터를 닫는다.
 * 7. 파일 디스크립터 0, 1, 2를 /dev/null로 리다이렉트한다.
**/

#define _POSIX_C_SOURCE  200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>

void unix_error(const char *msg);

void daemonize(const char *cmd)
{
    int fd0, fd1, fd2;
    struct rlimit rl;
    struct sigaction sa;
    pid_t pid;

    /* 1. umask를 0으로 변경한다. */
    umask(0);

    /* 프로세스가 열 수 있는 최대 파일 수를 얻어온다. */
    if (getrlimit(RLIMIT_OFILE, &rl) < 0)
        unix_error("getrlimit( RLIMIT_OFILE )");

    /* 2. 자식 프로세스 생성 후 부모 프로세스 종료한다. */
    if ((pid = fork()) < 0)
        unix_error("fork");
    else if (pid > 0)   /* parent process */
        exit(EXIT_SUCCESS);

    /* child process */

    /* 3. 세로운 세션을 만들고 리더가 된다. */
    if (setsid() < 0)
        unix_error("setsid");

    /* 4. SIGHUP을 무시하도록 sigaction을 설정한다. */
    sa.sa_handler = SIG_IGN;
    if (sigemptyset(&sa.sa_mask) < 0)
        unix_error("sigemptyset");
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0)
        unix_error("sigaction");

    /* 5. 작업 디렉터리를 루트 디렉토리로 변경한다. */
    if (chdir("/") < 0)
        unix_error("chdir");

    /* 6. 열려있는 파일 디스크립터를 닫는다. */
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;

    for(int i = 0; i < rl.rlim_max; i++)
        close(i);

    /* 7. 파일 디스크립터 0, 1, 2를 /dev/null로 리다이렉트한다. */
    fd0 = open("/dev/null", O_RDWR);  /* 표준 입력 */
    fd1 = dup(0);                     /* 표준 출력 */
    fd2 = dup(0);                     /* 표준 에러 */ 

    openlog(cmd, LOG_PID, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        syslog(LOG_NOTICE, " unexpected fd : %d %d %d", fd0, fd1, fd2);
        exit(EXIT_FAILURE);
    }
    
    syslog(LOG_NOTICE, "Hello, Daemon [%s]Start!\n", cmd);
}


int main(int argc, char *argv[])
{
    daemonize(argv[0]);

    /* 데몬에서 수행할 작업 작성 */
    sleep(180);

    closelog();
    exit(EXIT_SUCCESS);
}


void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}