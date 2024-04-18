/**
 * Reference
 * [1] https://tttsss77.tistory.com/278
 * 
 * timer-use-thread.c
 * 
 * 스레드 방식을 사용한 타이머 예제 입니다.
**/

#define _POSIX_C_SOURCE 200809L

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

/* 타이머 만료 시 실행되는 스레드 */
void timer_expired_thread(union sigval arg)
{
    /** 
     * TODO 
     * 시그널(인터럽트) 처리 함수가 아닌 쓰레드 함수이므로 
     * 뮤텍스 등 프로세스 컨텍스트에서 사용할 수 있는 지연 동작을 사용할 수 있다. 
    **/

    printf("Timer expired: %s\n", date());
}


int main(void) 
{
    struct itimerspec rt_itspec;
    memset(&rt_itspec, 0, sizeof(rt_itspec));
    /* 최초 타이머 만기 주기를 설정한다. */
    rt_itspec.it_value.tv_sec = INITIAL_DELAY / 1000;
    rt_itspec.it_value.tv_nsec = (INITIAL_DELAY % 1000) * 1000000;

    /* 두번째부터의 타이머 주기를 설정한다. */
    rt_itspec.it_interval.tv_sec = INTERVAL / 1000;
    rt_itspec.it_interval.tv_nsec = (INTERVAL % 1000) * 1000000;

    /* 타이머를 생성한다 - 타이머 만기 시 쓰레드가 생성되도록 설정한다 (SIGEV_THREAD). */
    timer_t timer;
    struct sigevent sigev;
    memset(&sigev, 0, sizeof(sigev));
    sigev.sigev_notify = SIGEV_THREAD;
    sigev.sigev_value.sival_ptr = &timer;
    sigev.sigev_notify_function = timer_expired_thread;
    sigev.sigev_notify_attributes = NULL;
    if (timer_create(CLOCK_MONOTONIC, &sigev, &timer) < 0)
        unix_error("timer_create");

    /* 타이머에 주기를 설정한다. */
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
