/**
 * Reference
 * [1] https://stackoverflow.com/questions/15458438/implementing-traceroute-using-icmp-in-c
 * [2] https://ko.wikipedia.org/wiki/IPv4
 * [3] https://ko.wikipedia.org/wiki/%EC%9D%B8%ED%84%B0%EB%84%B7_%EC%A0%9C%EC%96%B4_%EB%A9%94%EC%8B%9C%EC%A7%80_%ED%94%84%EB%A1%9C%ED%86%A0%EC%BD%9C
 * 
 * traceroute.c 
 * 
 * raw 소켓을 이용해서 목적지 주소로 이동하기 위한 경로들을 출력하는 프로그램 입니다.
**/

/**
 *     IP Header    
 * 
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |Version|  IHL  |Type of Service|          Total Length         |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |         Identification        |Flags|      Fragment Offset    |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |  Time to Live |    Protocol   |         Header Checksum       |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                       Source Address                          |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                    Destination Address                        |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  20 bytes
 *    |                    Options                    |    Padding    |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  0 ~ 40 bytes
 *    |                            Data                               |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  up to 65515 bytes
 * 
 *    ICMP Header
 *    
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |     Type      |     Code      |          Checksum             |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+  8 bytes
 *    |                          Contents                             |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 **/

#define _DEFAULT_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/select.h>

#include <arpa/inet.h>          /* inet_pton() */
#include <netinet/ip.h>         /* struct ip */
#include <netinet/ip_icmp.h>    /* struct icmp */

#include <netdb.h>
#include <unistd.h>

#define SRC_IP           "192.168.1.13"
#define RECV_TIMEOUT     1000  /* ms */
#define MAX_HOPS         30

#define ICMP_HEADER_LEN  8

#define IP_HEADER_LEN    20
#define IP_DATA_LEN      ICMP_HEADER_LEN
#define IP_PACKET_SIZE   (IP_HEADER_LEN + IP_DATA_LEN)

#define PACKET_SIZE      1440

void unix_error(const char *msg);
void gai_error(const char *msg, const int rc);

unsigned short chksum(unsigned short *data, int len);
struct timespec diff_timespec(const struct timespec *ts1, const struct timespec *ts2);
int recv_timeout(int fd, void *buffer, size_t n, int flags, int timeout_ms);


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <destination>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t pid = getpid();   /* identification */

    int rc_gai;
    struct addrinfo hints, *ai_dest;
    char src_ip[INET6_ADDRSTRLEN] = SRC_IP;
    char dest_ip[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_flags = AI_ADDRCONFIG;

    if ((rc_gai = getaddrinfo(argv[1], NULL, &hints, &ai_dest)))
        gai_error("getaddrinfo", rc_gai);

    if ((rc_gai = getnameinfo(ai_dest->ai_addr, ai_dest->ai_addrlen, dest_ip, sizeof(dest_ip),  NULL, 0, NI_NUMERICHOST)))
        gai_error("getnameinfo", rc_gai);

    printf("traceroute to %s (%s), %d hops max, %d byte packets\n", argv[1], dest_ip, MAX_HOPS, IP_PACKET_SIZE);

    /* raw 소켓 생성 */
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        unix_error("socket");

    /* ip 헤더를 직접 설정하기 위해 이 옵션을 활성화 한다. */
    int on = 1;
    if ((setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on))) < 0)
        unix_error("setsockopt( IP_HDRINCL )");


    int hop = 1;
    char send_packet[PACKET_SIZE];
    char recv_packet[PACKET_SIZE];
    struct ip *ip = NULL;
    struct icmp *icmp = NULL;
    struct timespec ts_send, ts_recv, ts_measure;
    while (hop < MAX_HOPS)
    {
        memset(send_packet, 0, sizeof(send_packet));
        memset(recv_packet, 0, sizeof(recv_packet));


        /* ip 헤더 작성 */
        ip = (struct ip *)send_packet;
        ip->ip_v = 4;       /* version */
        ip->ip_hl = 5;      /* header len */
        ip->ip_tos = 0;     /* type of service */
        ip->ip_len = htons(IP_PACKET_SIZE);     /* total length: 28 */
        ip->ip_id = htons(pid);                 /* identification */
        ip->ip_off = 0;             /* fragment offset field */
        ip->ip_ttl = hop;           /* time to live */
        ip->ip_p = IPPROTO_ICMP;    /* protocol */
        ip->ip_sum = 0;             /* check sum */
        /* source address */
        if (inet_pton(AF_INET, src_ip, &ip->ip_src.s_addr) < 0)
            unix_error("inet_pton");
        /* dest address */
        if (inet_pton(AF_INET, dest_ip, &ip->ip_dst.s_addr) < 0)
            unix_error("inet_pton");
        /* ip 체크섬 계산 */
        ip->ip_sum = chksum((unsigned short *)ip, IP_HEADER_LEN);


        /* icmp 헤더 작성 */
        icmp = (struct icmp *)(send_packet + IP_HEADER_LEN);
        icmp->icmp_type = ICMP_ECHO;    /* type of message, see below */
        icmp->icmp_code = 0;            /* type sub code */
        icmp->icmp_cksum = 0;           /* ones complement checksum of struct */
        icmp->icmp_id = pid;            /* id */
        icmp->icmp_seq = hop + 1;       /* seq number */
        /* icmp 체크섬 계산 */
        icmp->icmp_cksum = chksum((unsigned short *)icmp, ICMP_HEADER_LEN);

        /* 송신시간 얻기 */
        if (clock_gettime(CLOCK_MONOTONIC, &ts_send) < 0)
            unix_error("clock_gettime");

        /* send packet */
        if (sendto(sockfd, send_packet, IP_PACKET_SIZE, 0, ai_dest->ai_addr, ai_dest->ai_addrlen) < 0)
            unix_error("sendto");
        
        /* recv packet */
        int n_recv;
        if ((n_recv = recv_timeout(sockfd, recv_packet, sizeof(recv_packet), 0, RECV_TIMEOUT)) < 0)
        {
            if (errno)
                unix_error("recvfrom");

            printf("timeout!\n");
        }
        else
        {
            /* 수신 시간 얻기 */
            if (clock_gettime(CLOCK_MONOTONIC, &ts_recv) < 0)
                unix_error("clock_gettime");

            /* unpacking */
            ip = (struct ip *)recv_packet;
            int ip_header_len = (ip->ip_hl << 2);

            /* 수신된 ip주소 추출 */
            char recv_ip[INET6_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &ip->ip_src, recv_ip, sizeof(recv_ip)) < 0)
                unix_error("inet_ntop");

            if (ip->ip_p == IPPROTO_ICMP)
            {
                struct icmp *icmp = (struct icmp *)(recv_packet + ip_header_len);
                int icmp_len = n_recv - ip_header_len;

                if (icmp_len < 8)
                {
                    printf("icmp len: %d\n", icmp_len);
                    exit(EXIT_FAILURE);
                }

                /* 시간 계산 */
                ts_measure = diff_timespec(&ts_recv, &ts_send);
                double time = ts_measure.tv_sec * 1000.0 + ts_measure.tv_nsec / 1000000.0;
                printf("hop: %d\trecv ip: %s\t%.3lf ms\n", hop, recv_ip, time);

                if (icmp->icmp_type == ICMP_ECHOREPLY || icmp->icmp_type == ICMP_DEST_UNREACH)
                {
                    break;
                }
            }
            else
            {
                printf("ip protocol: %d\n", ip->ip_p);
                exit(EXIT_FAILURE);
            }
        }

        hop++;
    }

    if (close(sockfd) < 0)
        unix_error("close");
    freeaddrinfo(ai_dest);

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


unsigned short chksum(unsigned short *data, int len)
{
    /**
     * 체크섬 계산 방법
     * 1. 데이터를 2byte 단위로 나눠서 더한다. (odd byte의 경우 뒤를 0으로 패딩) 
     * 2. 덧샘 결과가 2byte를 초과하면 올림수(carry)를 하위 바이트에 더한다. 
     *    ex) 2DE46 -> DE46 + 0002 = DE48
     * 3. 덧샘 결과에 1의 보수를 취한다. (0은 1ㄹ로, 1은 10으로 치환)
    **/
    unsigned long sum = 0;
    unsigned short *w = data;   /* temp */
    
    /* 2byte 단위의 합 */
    while (len > 1)
    {
        sum += *w++;
        len -= 2;
    }

    /* 홀수 바이트 처리 패딩 */
    if (len == 1)
        sum += *(unsigned char *)w;

    /* 올림수 처리 */
    while (sum >> 16)
        sum = (sum >> 16) + (sum & 0xFFFF);

    /* 1의 보수 처리 */
    return (unsigned short)~sum;
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

int recv_timeout(int fd, void *buffer, size_t n, int flags, int timeout_ms)
{
    /**
     * 1s = 1000 ms = 1000000us
    **/
    long sec = timeout_ms / 1000;
    long usec = (timeout_ms % 1000) * 1000;
    
    fd_set set;

    FD_ZERO(&set);
    FD_SET(fd, &set);
    
    struct timeval tv = { .tv_sec = sec, .tv_usec = usec };
    if (select(fd + 1, &set, NULL, NULL, &tv) < 0)
        unix_error("select");
    
    if (FD_ISSET(fd, &set))
        return (int)recvfrom(fd, buffer, n, flags, NULL, NULL);

    return -1;  /* time out */
}
