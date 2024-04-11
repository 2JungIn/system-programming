/**
 * packet-capture.c
 * 
 * raw 소켓을 사용해서 패킷을 캡쳐하는 프로그램입니다. 
**/

#define _POSIX_C_SOURCE  200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>      /* ntons() */
#include <net/ethernet.h>   /* ETH_P_ALL */

#define BUF_SIZE 4096

#define DEFAULT_FLAG  (O_WRONLY | O_CREAT | O_TRUNC)
#define DEFAULT_MODE  (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)

void unix_error(const char *msg);

int main(void) {
    int raw_sock;
    unsigned char buf[BUF_SIZE];
    ssize_t n_read;

    if ((raw_sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)  /* error: socket() */
        unix_error("socket");

    if ((n_read = read(raw_sock, buf, sizeof(buf))) < 0) /* error: read() */
    {
        close(raw_sock);
        unix_error("read");
    }

    if (close(raw_sock) < 0)
        unix_error("close");

    printf("%ld byte read!\n", n_read);

    int fd;
    if ((fd = open("captured-packet.txt", DEFAULT_FLAG, DEFAULT_MODE)) < 0)  /* error: open() */
        unix_error("open");

    for (ssize_t i = 0; i < n_read; i++)
    {
        int cnt = i + 1;
        dprintf(fd, "%02x", buf[i]);
        if (cnt % 16 == 0)
            dprintf(fd, "\n");
        else if (cnt % 8 == 0)
            dprintf(fd, "    ");
        else
            dprintf(fd, " ");
    }

    if (close(fd) < 0)
        unix_error("close");
    
    exit(EXIT_SUCCESS);
}

void unix_error(const char *msg)    /* error handling function */
{
    perror(msg);
    exit(EXIT_FAILURE);
}