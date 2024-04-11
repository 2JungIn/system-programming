/**
 * dnslookup.c 
 * 
 * - 도메인 주소의 IP주소를 출력합니다.
**/

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

void gai_error(const char *msg, const int rc_gai);    /* error handling function */

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usagi: %s <domain name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints, *ai;
    int rc_gai; /* getaddrinfo(), getnameinfo() return code */

    /* hints 초기화 */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        /* IPv4, IPv6 주소 모두 선택한다. */
    hints.ai_socktype = SOCK_STREAM;    /* TCP 소켓 */
    hints.ai_flags = AI_ADDRCONFIG;     /* 플래그 지정 */

    if ((rc_gai = getaddrinfo(argv[1], NULL, &hints, &ai)))
        gai_error("getaddrinfo", rc_gai);

    printf("Name: %s\n", argv[1]);
    char host[INET6_ADDRSTRLEN];
    for (struct addrinfo *curr = ai; curr; curr = curr->ai_next)
    {
        if ((rc_gai = getnameinfo(
            curr->ai_addr, curr->ai_addrlen, 
            host, sizeof(host), NULL, 0,
            NI_NUMERICHOST)))
        {
            /* error */
            gai_error("getnameinfo", rc_gai);
        }
        printf("Address: %s\n", host);
    }
    
    freeaddrinfo(ai);

    exit(EXIT_SUCCESS);
}

void gai_error(const char *msg, const int rc_gai)    /* error handling function */
{
    fprintf(stderr, "%s: %s\n", msg, gai_strerror(rc_gai));
    exit(EXIT_FAILURE);
}