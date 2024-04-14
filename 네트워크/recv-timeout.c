/**
 * Reference
 * [1] https://softtone-someday.tistory.com/17
 * 
 * recv_timeout.c
 * 
 * I/O 멀티플랙싱 함수인 select()를 사용해서 recv() 함수의 동작에서 타임아웃 기능을 구현한 프로그램 입니다.
 * 클라이언트로 부터 5초 이상 아무런 응답이 없으면 연결이 끊깁니다.
**/

#define _POSIX_C_SOURCE  200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/select.h>

#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>          /* struct addrinfo, getaddrinfo(), getnameinfo() */

#include <netinet/tcp.h>    /* TCP_NODELAY */

#define BACKLOG       256
#define DEFAULT_PORT  "56562"
#define PORTSTRLEN    sizeof("65535")
#define BUFFER_SIZE   1024
#define RECV_TIMEOUT  5000  /* ms */

void unix_error(const char *msg);

int setsockopt_reuseaddr(int fd);
int setsockopt_tcpnodelay(int fd);
int setsockopt_linger(int fd);

int make_listen_socket(const char *host, const char *port);


ssize_t recv_timeout(int fd, void *buffer, size_t n, int flags, int timeout_ms)
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
        return recv(fd, buffer, n, flags);

    return -1;  /* time out */
}


int main(void)
{
    int fd_listener = make_listen_socket(NULL, DEFAULT_PORT);

    int cfd;
    char buffer[BUFFER_SIZE];
    ssize_t n_recv, n_send;
    while (1)
    {
        if ((cfd = accept(fd_listener, NULL, NULL)) < 0)
        {
            if (errno == EINTR)
                continue;
            
            unix_error("accept");
        }

        /* client connected */
        printf("client connected: cfd(%d), timeout(%dms)\n", cfd, RECV_TIMEOUT);
        while (1)
        {
            memset(buffer, 0, sizeof(buffer));

            if ((n_recv = recv_timeout(cfd, buffer, sizeof(buffer), 0, RECV_TIMEOUT)) < 0)
            {
                if (errno)
                {
                    if (errno == EINTR)
                        continue;
                    else
                        perror("recv");
                }
                else
                {
                    printf("recv timeout!\n");
                }
                
                break;
            }
            else if (n_recv == 0)   /* connection close */
            {
                break;
            }
            else    /* echo */
            {
                printf("cfd(%d) recv msg: %*s", cfd, (int)n_recv, buffer);
                if ((n_send = send(cfd, buffer, n_recv, 0)) < 0)
                {
                    if (errno == EINTR)
                        continue;

                    perror("send");
                    break;
                }
            }
        }
        printf("client connection closed!\n");

        setsockopt_linger(cfd);
        if (close(cfd) < 0)
            unix_error("close");
    }

    if (close(fd_listener) < 0)
        unix_error("close");

    return 0;
}


void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int setsockopt_reuseaddr(int fd)
{
    int on = 1;
    if ((setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on))) < 0)
    {
        perror("setsockopt( SO_REUSEADDR )");
        return -1;
    }

    return 0;
}

int setsockopt_tcpnodelay(int fd)
{
    int on = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) < 0)
    {
        perror("setsockopt( TCP_NODELAY )");
        return -1;
    }

    return 0;
}

int setsockopt_linger(int fd)
{
    struct linger l = {.l_onoff = 1, .l_linger = 0};
    if ((setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l))) < 0)
    {
        perror("setsockopt( SO_LINGER )");
        return -1;
    }

    return 0;
}

int make_listen_socket(const char *host, const char *port)
{
    struct addrinfo hints, *ai;
    int rc_gai;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG | AI_PASSIVE;

    if ((rc_gai = getaddrinfo(host, port, &hints, &ai))) /* error: gaddrinfo() */
    {
        fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(rc_gai));
        exit(EXIT_FAILURE);
    }

    int fd_listener = -1;
    for (struct addrinfo *curr = ai; curr; curr = curr->ai_next)
    {
        if ((fd_listener = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol)) < 0)   /* error: socket() */
        {
            perror("socket");
            continue;
        }

        /* reuse address */
        if (setsockopt_reuseaddr(fd_listener) < 0)   /* error: setsockopt_reuseaddr() */
        {
            close(fd_listener);
            continue;
        }

        /* tcp no delay */
        if (setsockopt_tcpnodelay(fd_listener) < 0)
        {
            close(fd_listener);
            continue;
        }

        if (bind(fd_listener, curr->ai_addr, curr->ai_addrlen) < 0)  /* error: bind() */
        {
            perror("bind");
            close(fd_listener);
            continue;
        }

        if (listen(fd_listener, BACKLOG) < 0)    /* error: listen() */
        {
            perror("listen");
            close(fd_listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai);

    struct sockaddr_storage saddr_s;
    socklen_t len_saddr_s = sizeof(saddr_s);
    if (getsockname(fd_listener, (struct sockaddr *)&saddr_s, &len_saddr_s) < 0)
    {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }
    
    char bind_addr[INET6_ADDRSTRLEN];
    char bind_port[PORTSTRLEN];
    if ((rc_gai = getnameinfo(
        (struct sockaddr *)&saddr_s, len_saddr_s, 
        bind_addr, sizeof(bind_addr),
        bind_port, sizeof(bind_port),
        NI_NUMERICHOST | NI_NUMERICSERV)))
    {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(rc_gai));
        exit(EXIT_FAILURE);
    }
    printf("Server opened: (%s:%s)\n", bind_addr, bind_port);

    return fd_listener;
}
