#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

void unix_error(const char *msg);


int main(void)
{
    sleep(10);
    
    struct rusage usage;
    /**
     * Reference:
     *  [1] https://wariua.github.io/man-pages-ko/getrusage%282%29/
     * 
     *  RUSAGE_SELF:
     *   - 호출 프로세스의 프로세스 내 모든 
     *     스레드의 자원 사용 합계를 반환한다.
     * RUSAGE_CHILDREN
     *   - 종료돼서 대기까지 이뤄진 호출 프로세스의 
     *     자식 모두에 대한 자원 사용 통계를 반환한다. 
     * RUSAGE_THREAD 
     *   - 호출 스레드의 자원 사용 통계를 반환한다. 
    **/
    if (getrusage(RUSAGE_SELF, &usage) < 0)
        unix_error("getrusage");

    printf("user CPU time used: %ld.%lds\n", usage.ru_utime.tv_sec, usage.ru_utime.tv_usec);
    printf("system CPU time used: %ld.%lds\n", usage.ru_stime.tv_sec, usage.ru_stime.tv_usec);
    printf("maximum resident set size: %ld\n", usage.ru_maxrss);
    printf("integral shared memory size: %ld\n", usage.ru_ixrss);
    printf("integral unshared data size: %ld\n", usage.ru_idrss);
    printf("integral unshared stack size: %ld\n", usage.ru_isrss);
    printf("page reclaims (soft page faults): %ld\n", usage.ru_minflt);
    printf("page faults (hard page faults): %ld\n", usage.ru_majflt);
    printf("swaps: %ld\n", usage.ru_nswap);
    printf("block input operations: %ld\n", usage.ru_inblock);
    printf("block output operations: %ld\n", usage.ru_oublock);
    printf("IPC messages sent: %ld\n", usage.ru_msgsnd);
    printf("IPC messages received: %ld\n", usage.ru_msgrcv);
    printf("signals received: %ld\n", usage.ru_nsignals);
    printf("voluntary context switches: %ld\n", usage.ru_nvcsw);
    printf("involuntary context switches: %ld\n", usage.ru_nivcsw);

    exit(EXIT_SUCCESS);
}

void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}