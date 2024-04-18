/**
 * Reference
 * [1] https://tttsss77.tistory.com/278
 * 
 * timer-use-signal.c
 * 
 * 시그널 방식을 사용한 타이머 예제 입니다.
**/

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <signal.h>

#define INITIAL_DELAY   1000    /* 최초 타이머 만기 지연 (밀리초 단위) */
#define INTERVAL        1000    /* 타이머 만기 주기 (밀리초 단위) */

#define BUF_SIZE        50

void unix_error(const char *msg);
char *date(void);

/* SIGRTMIN 시그널 핸들러 함수 */
void sa_sigaction_rtmin(int signum, siginfo_t *si, void *sv)
{
    printf("Timer expired: %s\n", date());
}


int main(void) 
{
    struct sigaction sa_rt;
    memset(&sa_rt, 0, sizeof(sa_rt));
    if (sigemptyset(&sa_rt.sa_mask) < 0)
        unix_error("sigemptyset");
    sa_rt.sa_sigaction = sa_sigaction_rtmin;
    sa_rt.sa_flags = SA_SIGINFO | SA_RESTART;

    if (sigaction(SIGRTMIN, &sa_rt, NULL) < 0)
        unix_error("sigaction");

    struct sigevent sigev;
    memset(&sigev, 0, sizeof(sigev));
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGRTMIN;

    /* 타이머를 생성한다 - 타이머 만기 시 SIGRTMIN시그널이 발생 하도록 설정한다 (SIGEV_SIGNAL). */
    timer_t timer;
    if (timer_create(CLOCK_MONOTONIC, &sigev, &timer) < 0)
        unix_error("timer_create");

    /* 타이머에 주기를 설정한다. */
    struct itimerspec rt_itspec;
    memset(&rt_itspec, 0, sizeof(rt_itspec));
    /* 최초 타이머 만기 주기를 설정한다. */
    rt_itspec.it_value.tv_sec = INITIAL_DELAY / 1000;
    rt_itspec.it_value.tv_nsec = (INITIAL_DELAY % 1000) * 1000000;

    /* 두번째부터의 타이머 주기를 설정한다. */
    rt_itspec.it_interval.tv_sec = INTERVAL / 1000;
    rt_itspec.it_interval.tv_nsec = (INTERVAL % 1000) * 1000000;
    if (timer_settime(timer, 0, &rt_itspec, NULL) < 0)
        unix_error("timer_settime");

    printf("timer_settimer(): %s\n", date());

    printf("엔터를 누르면 종료 합니다.\n");
    
    getchar();

    if (timer_delete(timer) < 0)
        unix_error("timer_delete");

    exit(EXIT_SUCCESS);
}


void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

char *date(void)
{
    static char buffer[BUF_SIZE];

    time_t now = time(NULL);
    struct tm *tmp = NULL;
    if ((tmp = localtime(&now)) == NULL) /* error: localtime() */
    {
        perror("localtime");
        return NULL;
    }
    
    if (strftime(buffer, sizeof(buffer), "%Y. %m. %d. (%a) %T %Z(GMT%z)", tmp) == 0)  /* error: strftime() */
    {
        fprintf(stderr, "strftime() returned 0!\n");
        return NULL;
    }

    return buffer;
}
