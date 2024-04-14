/**
 * Reference
 * [1] https://blog.naver.com/barcel/221222325295
 * [2] https://datatracker.ietf.org/doc/html/rfc1071
 * [3] http://www.ktword.co.kr/test/view/view.php?no=1477
 * [4] https://noodles8436.tistory.com/13
 * 
 * internet-checksum.c
 * rfc1071의 체크섬 함수를 구현한 프로그램 입니다.
**/

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>  /* inet_pton() */

#include <netinet/ip.h>         /* struct ip */
#include <netinet/ip_icmp.h>    /* struct icmp */
#include <netinet/tcp.h>        /* struct tcp */

#define TCP_HEADER_LEN   20

#define ICMP_HEADER_LEN  8
#define ICMP_DATA_LEN    56

#define IP_HEADER_LEN    20

#define PACKET_SIZE      1440

struct pseudo_header
{
    uint32_t src_addr;  /* src address */
    uint32_t dest_addr; /* dest address */
    uint8_t placeholder;   /* only 0 */
    uint8_t protocol;   /* ip protocol (IPPROTO_TCP, IPPROTO_UDP ...) */
    uint16_t tcp_segment_length;    /* tcp header + tcp data (byte) */
};

void unix_error(const char *msg);
void print_byte(const char *byte, const size_t n);

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

void Test01(void);
void Test02(void);


int main(void)
{
    Test01();   /* ip + icmp */
    Test02();   /* ip + tcp */

    exit(EXIT_SUCCESS);
}


void Test01(void)
{
    printf("[Test01]\n");
    /**
     * ICMP Echo Request Packet
     * 0000   45 00 00 54 6f 67 40 00     40 01 f9 7c c0 a8 01 0d
     * 0010   08 08 08 08 08 00 44 72     3e 84 06 00 36 23 00 00 
     * 0020   00 00 00 00 a4 cb 94 1a     00 00 00 00 00 00 00 00 
     * 0030   00 00 00 00 00 00 00 00     00 00 00 00 00 00 00 00 
     * 0040   00 00 00 00 00 00 00 00     00 00 00 00 00 00 00 00 
     * 0050   00 00 00 00
     *
     * 
     * ICMP Echo Replay Packet
     * 
     * 0000   45 00 00 54 00 00 00 00     71 01 77 e4 08 08 08 08
     * 0010   c0 a8 01 0d 00 00 4c 72     3e 84 06 00 36 23 00 00 
     * 0020   00 00 00 00 a4 cb 94 1a     00 00 00 00 00 00 00 00 
     * 0030   00 00 00 00 00 00 00 00     00 00 00 00 00 00 00 00 
     * 0040   00 00 00 00 00 00 00 00     00 00 00 00 00 00 00 00 
     * 0050   00 00 00 00
    **/
    char icmp_echo_data[ICMP_DATA_LEN] = 
    {
        0x36, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0xa4, 0xcb, 0x94, 0x1a, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    
    char packet[PACKET_SIZE];
    memset(packet, 0, sizeof(packet));

    unsigned short packet_len = IP_HEADER_LEN + ICMP_HEADER_LEN + ICMP_DATA_LEN;
    /* IPv4 헤더 작성 */
    struct ip *ip = (struct ip *)packet;
    ip->ip_v = 4;             /* version */
    ip->ip_hl = 5;            /* header length */
    ip->ip_tos = 0;           /* type of service */
    ip->ip_len = htons(packet_len);   /* total length */
    ip->ip_id = htons(0);     /* identification */
    ip->ip_off = htons(0);    /* fragment offset field */
    ip->ip_ttl = 113;         /* time to live */
    ip->ip_p = IPPROTO_ICMP;  /* protocol */
    ip->ip_sum = 0;
    /* src address */
    if (inet_pton(AF_INET, "8.8.8.8", &ip->ip_src.s_addr) < 0)
        unix_error("inet_pton");
    /* dest address */
    if (inet_pton(AF_INET, "192.168.1.13", &ip->ip_dst.s_addr) < 0)
        unix_error("inet_pton");
    /* 체크섬 계산 */
    ip->ip_sum = chksum((unsigned short *)ip, IP_HEADER_LEN);

    /* icmp 헤더 작성 */
    struct icmp *icmp = (struct icmp *)(packet + IP_HEADER_LEN);
    icmp->icmp_type = 0;    /* icmp type (Echo (ping) replay) */
    icmp->icmp_code = 0;    /* icmp code */
    icmp->icmp_cksum = 0;   /* icmp checksum */
    icmp->icmp_id = 33854;  /* icmp id */
    icmp->icmp_seq = 6;     /* icmp seq number */
    /* icmp data */
    memcpy(icmp->icmp_data, icmp_echo_data, ICMP_DATA_LEN);
    /* icmp 체크섬 계산 */
    icmp->icmp_cksum = chksum((unsigned short *)icmp, ICMP_HEADER_LEN + ICMP_DATA_LEN);

    print_byte(packet, packet_len);

    printf("[checksum]\n");
    printf("ip: %d\n", chksum((unsigned short *)ip, IP_HEADER_LEN));
    printf("icmp: %d\n", chksum((unsigned short *)icmp, ICMP_HEADER_LEN + ICMP_DATA_LEN));
}

void Test02(void)
{
    printf("[Test02]\n");
    /**
     * TCP Syn Packet
     * 
     * 0000   45 10 00 3c c6 20 40 00     40 06 76 89 7f 00 00 01
     * 0010   7f 00 00 01 cf 6e dc f2     5b 81 e9 c5 00 00 00 00 
     * 0020   a0 02 ff d7 fe 30 00 00     02 04 ff d7 04 02 08 0a 
     * 0030   71 4b c3 bf 00 00 00 00     01 03 03 07
    **/

    char packet[PACKET_SIZE];
    memset(packet, 0, sizeof(packet));

    
    int tcp_option_len = TCPOLEN_MAXSEG             /* Maximum segment size */
                        + TCPOLEN_SACK_PERMITTED    /* SACK permitted */
                        + TCPOLEN_TIMESTAMP         /* Timestamps: TSval */
                        + 1     /* Nop */
                        + TCPOLEN_WINDOW;   /* Window scale */
    uint16_t tcp_header_len = (TCP_HEADER_LEN + tcp_option_len);
    uint16_t data_size = 0;
    uint16_t packet_len = IP_HEADER_LEN + tcp_header_len + data_size;


    /* IPv4 헤더 작성 */
    struct ip *ip = (struct ip *)packet;
    ip->ip_v = 4;     /* version */
    ip->ip_hl = 5;    /* header length */
    ip->ip_tos = IPTOS_LOWDELAY;       /* type of service */
    ip->ip_len = htons(packet_len);    /* total length */
    ip->ip_id = htons(0xc620);    /* identification */
    ip->ip_off = htons(IP_DF);    /* fragment offset field */
    ip->ip_ttl = 64;         /* time to live */
    ip->ip_p = IPPROTO_TCP;  /* protocol */
    ip->ip_sum = 0;
    /* src address */
    if (inet_pton(AF_INET, "127.0.0.1", &ip->ip_src.s_addr) < 0)
        unix_error("inet_pton");
    /* dest address */
    if (inet_pton(AF_INET, "127.0.0.1", &ip->ip_dst.s_addr) < 0)
        unix_error("inet_pton");
    /* 체크섬 계산 */
    ip->ip_sum = chksum((unsigned short *)ip, IP_HEADER_LEN);


    /* TCP 헤더 작성 */
    struct tcphdr *tcp = (struct tcphdr *)(packet + IP_HEADER_LEN);
    tcp->th_sport = htons(53102);   /* source port */
    tcp->th_dport = htons(56562);   /* destination port */
    tcp->th_seq = htonl(0x5b81e9c5);        /* sequence number */
    tcp->th_ack = htonl(0);                 /* acknowledgement number */
    tcp->th_off = tcp_header_len >> 2;      /* header length */
    tcp->th_flags = TH_SYN;         /* flags */
    tcp->th_win = htons(65495);     /* window */
    tcp->th_sum = 0;                /* checksum */
    tcp->th_urp = htons(0);         /* urgent pointer */

    char *tcp_option = (packet + IP_HEADER_LEN + TCP_HEADER_LEN);
    
    /* option 1: TCP Option - Maximum segment size: 65495 bytes */
    tcp_option[0] = TCPOPT_MAXSEG;
    tcp_option[1] = TCPOLEN_MAXSEG;
    *(uint16_t *)&tcp_option[2] = htons(65495);
    tcp_option += TCPOLEN_MAXSEG;   /* next */

    /* option 2: TCP Option - SACK permitted */
    tcp_option[0] = TCPOPT_SACK_PERMITTED;
    tcp_option[1] = TCPOLEN_SACK_PERMITTED;
    tcp_option += TCPOLEN_SACK_PERMITTED;   /* next */
    
    /* option 3: TCP Option - Timestamps: TSval 1900790719, TSecr 0 */
    tcp_option[0] = TCPOPT_TIMESTAMP;
    tcp_option[1] = TCPOLEN_TIMESTAMP;
    *(uint32_t *)&tcp_option[2] = htonl(0x714bc3bf);
    *(uint32_t *)&tcp_option[6] = htonl(0);
    tcp_option += TCPOLEN_TIMESTAMP;    /* next */

    /* option 4: TCP Option - No-Operation (NOP) */
    tcp_option[0] = TCPOPT_NOP;
    tcp_option += 1;    /* next */

    /* option 5: TCP Option - Window scale: 7 (multiply by 128) */
    tcp_option[0] = TCPOPT_WINDOW;
    tcp_option[1] = TCPOLEN_WINDOW;
    tcp_option[2] = 7;


    /* TCP 체크섬 계산을 위해 가상 헤더 작성 */
    int dummy_packet_len = sizeof(struct pseudo_header) + tcp_header_len + data_size;
    char dummy_packet[PACKET_SIZE];
    memset(dummy_packet, 0, sizeof(dummy_packet));

    struct pseudo_header *ph = (struct pseudo_header *)dummy_packet;
    ph->src_addr = ip->ip_src.s_addr;
    ph->dest_addr = ip->ip_dst.s_addr;
    ph->placeholder = 0;
    ph->protocol = ip->ip_p;
    ph->tcp_segment_length = htons(tcp_header_len + data_size);

    memcpy(dummy_packet + sizeof(struct pseudo_header), tcp, tcp_header_len + data_size);

    /* TCP 체크섬 계산 */
    tcp->th_sum = chksum((unsigned short *)dummy_packet, dummy_packet_len);

    print_byte(packet, packet_len);
    printf("%d\n", chksum((unsigned short *)ip, IP_HEADER_LEN));
    memcpy(dummy_packet + sizeof(struct pseudo_header), tcp, tcp_header_len + data_size);
    printf("%d\n", chksum((unsigned short *)dummy_packet, dummy_packet_len));
}


void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_byte(const char *byte, const size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        printf("%02x", (unsigned char)byte[i]);
        if ((i + 1) % 16 == 0)
            printf("\n");
        else if ((i + 1) % 8 == 0)
            printf("    ");
        else
            printf(" ");
    }

    printf("\n");
}
