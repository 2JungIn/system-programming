/**
 * time-measure.c
 * 
 *  clock_gettime() 함수를 사용해서 어떤 작업의 수행 시간을 측정하는 프로그램 입니다.
**/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <unistd.h>

void unix_error(const char *msg);

struct timespec diff_timespec(const struct timespec *ts1, const struct timespec *ts2);
unsigned long get_millisecond(const struct timespec *ts);


int main(void)
{
    struct timespec ts_start, ts_end, ts_measure;
    if (clock_gettime(CLOCK_MONOTONIC, &ts_start) < 0)
        unix_error("clock_gettime");

    /* 시간을 측정하고 싶은 작업 */
    struct timespec ts = { .tv_sec = 3, .tv_nsec = 500000000 };
    nanosleep(&ts, NULL);

    if (clock_gettime(CLOCK_MONOTONIC, &ts_end) < 0)
        unix_error("clock_gettime");

    ts_measure = diff_timespec(&ts_end, &ts_start);
    
    printf("%ldms\n", get_millisecond(&ts_measure));

    exit(EXIT_SUCCESS);
}


void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}


struct timespec diff_timespec(const struct timespec *ts1, const struct timespec *ts2)
{
    struct timespec ret_ts;

    ret_ts.tv_sec = ts1->tv_sec - ts2->tv_sec;
    ret_ts.tv_nsec = ts1->tv_nsec - ts2->tv_nsec;

    if (ret_ts.tv_nsec < 0)
    {
        ret_ts.tv_nsec += 1000000000;
        ret_ts.tv_sec--;
    }

    return ret_ts;
}

unsigned long get_millisecond(const struct timespec *ts)
{
    return ts->tv_sec * 1000 + ts->tv_nsec / 1000000;
}