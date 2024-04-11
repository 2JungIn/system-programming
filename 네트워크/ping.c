/**
 * Reference
 * [1] https://github.com/neelkanth13/ipv4-and-ipv6-raw-sockets/blob/master/icmpv4%20ping%20packet%20raw%20socket%20code.c
 * 
 * ping.c 
 * 
 * raw 소켓을 이용해서 핑을 구현한 프로그램 입니다.
**/

/**
 * ICMP Header
 *    
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |     Type      |     Code      |          Checksum             |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                          Contents                             |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
**/

#define _DEFAULT_SOURCE /* __USE_MISC */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>  /* DBL_MAX, DBL_MIN */

#include <sys/socket.h>

#include <netinet/ip.h>         /* struct ip */
#include <netinet/ip_icmp.h>    /* struct icmp */

#include <netdb.h>
#include <unistd.h>
#include <signal.h>

#define PACKET_SIZE     4096
#define ICMP_DATA_LEN   56


int sockfd;
int send_cnt, recv_cnt;

struct addrinfo *ai_dest;

struct timespec ts_start;
struct timespec ts_recv;

pid_t pid;
char *destination;
double rtt_min = DBL_MAX, rtt_max = DBL_MIN, rtt_total;


void unix_error(const char *msg);
void gai_error(const char *msg, const int rc);

__sighandler_t Signal(int sig, __sighandler_t handler);
void signal_handler(int signo);

unsigned short chksum(unsigned short *addr, int len);
struct timespec diff_timespec(const struct timespec *ts1, const struct timespec *ts2);
unsigned long get_millisecond(const struct timespec *ts);
int make_icmp_packet(char *packet, uint8_t type, int seq_no, int id);
int unpack_icmp_reply(char *packet, int len);
void send_icmp_echo_packet(void);
void recv_icmp_reply_packet(void);


int main(int argc,char *argv[])
{
    if (clock_gettime(CLOCK_MONOTONIC, &ts_start) < 0)
        unix_error("clock_gettime");

    Signal(SIGINT, signal_handler);
    Signal(SIGALRM, signal_handler);

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <destination>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid = getpid();
    destination = argv[1];

    int rc_gai;
    struct addrinfo hints;
    char host[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_ADDRCONFIG;

    if ((rc_gai = getaddrinfo(destination, NULL, &hints, &ai_dest)))
        gai_error("getaddrinfo", rc_gai);

    if ((rc_gai = getnameinfo(ai_dest->ai_addr, ai_dest->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST)))
        gai_error("getnameinfo", rc_gai);

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        unix_error("socket");

    printf("PING %s(%s): %d bytes data in ICMP packets.\n", destination, host, ICMP_DATA_LEN);
    
    if (kill(pid, SIGALRM) < 0)
        unix_error("kill");

    while (1)
        recv_icmp_reply_packet();

    exit(EXIT_SUCCESS);
}


void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void gai_error(const char *msg, const int rc)
{
    fprintf(stderr, "%s: %s\n", msg, gai_strerror(rc));
    exit(EXIT_FAILURE);
}


__sighandler_t Signal(int sig, __sighandler_t handler)
{
    struct sigaction action, old_action;

    action.sa_handler = handler;
    if (sigemptyset(&action.sa_mask) < 0)
        unix_error("sigemptyset");
    action.sa_flags = SA_RESTART;

    if (sigaction(sig, &action, &old_action) < 0)
    {
        perror("sigaction");
        return SIG_ERR;
    }

    return old_action.sa_handler;
}

void signal_handler(int signo)
{
    if (signo == SIGALRM)
    {
        send_icmp_echo_packet();
    }
    else if (signo == SIGINT)   /* ctrl + c */
    {
        struct timespec ts_end, ts_diff;
        if (clock_gettime(CLOCK_MONOTONIC, &ts_end) < 0)
            unix_error("clock_gettime");
        
        ts_diff = diff_timespec(&ts_end, &ts_start);

        double loss = (double)(send_cnt - recv_cnt) / send_cnt * 100.0;
        printf("\n");
        printf("\n-----------%s PING statistics-----------\n", destination);
        printf("%d packets transmitted, %d received, %2.0f%%  loss, time %ldms\n",
                send_cnt, recv_cnt, loss, get_millisecond(&ts_diff));

        if (recv_cnt)
        {
            double rtt_avg = rtt_total / recv_cnt;
            printf("rtt min/avg/max = %lf %lf %lf\n", rtt_min, rtt_avg, rtt_max);
        }

        if (close(sockfd) < 0)
            unix_error("close");

        freeaddrinfo(ai_dest);
        exit(EXIT_SUCCESS);
    }
    else
    {
        printf("Unkown signal: %d\n", signo);
    }
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

unsigned short chksum(unsigned short *addr, int len)
{
    int n_left = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (n_left > 1)
    {
        sum += *w++;
        n_left -= 2;
    }

    if (n_left == 1)
    {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}

int make_icmp_packet(char *packet, uint8_t type, int seq_no, int id)
{
    int packet_size = 8 + ICMP_DATA_LEN;

    /* ICMP헤더 작성 */
    struct icmp *icmp = (struct icmp *)packet;
    icmp->icmp_type = type;
    icmp->icmp_code = 0;
    icmp->icmp_cksum = 0;
    icmp->icmp_seq = seq_no;
    icmp->icmp_id = id;
    
    struct timespec *tspec = (struct timespec *)(icmp->icmp_data);
    if (clock_gettime(CLOCK_MONOTONIC, tspec) < 0)
        unix_error("clock_gettime");

    /* 체크섬 계산 */
    icmp->icmp_cksum = chksum((unsigned short *)icmp, packet_size);
    
    return packet_size;
}

int unpack_icmp_reply(char *packet, int len)
{
    struct ip *ip = (struct ip *)packet;
    int len_ip_heade = (int)(ip->ip_hl << 2);

    struct icmp *icmp = (struct icmp *)(packet + len_ip_heade);
    int len_icmp = len - len_ip_heade;

    if (len_icmp < 8)
    {
        printf("ICMP packets\'s length is less than 8\n");
        return -1;
    }

    /* icmp_type은 ICMP_ECHOREPLAY이어야 한다. */
    if (icmp->icmp_type != ICMP_ECHOREPLY)
        return -1;
    
    /* icmp_id는 pid와 같다. */
    if (icmp->icmp_id != pid)
        return -1;

    struct timespec *ts_send = (struct timespec *)icmp->icmp_data;

    /* 패킷을 송신한 시간과 패킷이 수신된 시간의 차이를 계산한다. */
    struct timespec ts_diff = diff_timespec(&ts_recv, ts_send);

    /* rtt를 ms 단위로 계산한다. */ 
    double rtt = ts_diff.tv_sec * 1000.0f + ts_diff.tv_nsec / 1000000.0f;

    rtt_min = (rtt < rtt_min ? rtt : rtt_min);
    rtt_max = (rtt > rtt_max ? rtt : rtt_max);
    rtt_total += rtt;

    printf("%d byte from %s: icmp_seq=%u ttl=%d rtt=%.3f ms\n",
            len_icmp, destination, icmp->icmp_seq, ip->ip_ttl, rtt);

    return 0;
}

void send_icmp_echo_packet(void)
{
    int packet_size;
    char packet[PACKET_SIZE] = { 0 };

    send_cnt++;
    packet_size = make_icmp_packet(packet, ICMP_ECHO, send_cnt, pid);

    if (sendto(sockfd, packet, packet_size, 0, ai_dest->ai_addr, ai_dest->ai_addrlen) < 0)
    {
        perror("sendto");
        send_cnt--;
    }

    alarm(1);
}

void recv_icmp_reply_packet(void)
{
    int n_recv;
    char packet[PACKET_SIZE] = { 0 };

    Signal(SIGALRM, signal_handler);

    while (recv_cnt < send_cnt)
    {
        if ((n_recv = (int)recvfrom(sockfd, packet, sizeof(packet), 0, NULL, NULL)) < 0)
        {
            if(errno == EINTR)
                continue;

            perror("recvfrom");
            continue;
        }

        if (clock_gettime(CLOCK_MONOTONIC, &ts_recv) < 0)
            unix_error("clock_gettime");

        if (unpack_icmp_reply(packet, n_recv) < 0)
            continue;

        recv_cnt++;
    }
}
