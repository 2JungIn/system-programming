/**
 * simple-copy.c
 * 
 * 메모리 매핑 함수인 mmap()을 활용하여 파일을 복사하는 프로그램 입니다.
**/

#define _XOPEN_SOURCE  700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <unistd.h>

void unix_error(const char *msg);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <src file> <dst file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    

    int src_fd, dst_fd; /* file descriptor of source file and destination file */
    struct stat s;      /* file state of source file */
    mode_t mode;        /* mode of source file */
    size_t src_size;    /* size of source file */
    char *src, *dst;    /* mapped address of source file, destination file */

    /* 복사할 파일 열기 */
    if ((src_fd = open(argv[1], O_RDONLY)) < 0)
        unix_error("open");
    
    /* 복사할 파일의 크기와 모드 얻기 */
    if (fstat(src_fd, &s) < 0)
    {
        close(src_fd);
        unix_error("fstat");
    }

    src_size = s.st_size;
    mode = s.st_mode;

    /* 복사할 파일 매핑 */
    if ((src = (char *)mmap(NULL, src_size, PROT_READ, MAP_SHARED, src_fd, 0)) == MAP_FAILED)
    {
        close(src_fd);
        unix_error("mmap");
    }

    /* 복사할 파일의 파일기술자 닫기 (매핑이 성공 했으므로 이제 파일기술자는 필요 없다.) */
    close(src_fd);


    /* 복사될 파일 열기 */
    if ((dst_fd = open(argv[2], O_CREAT | O_TRUNC | O_RDWR, mode)) < 0)
    {
        munmap(src, src_size);
        unix_error("open");
    }
    
    /* 복사될 파일의 크기를 복사할 파일의 크기까지 늘려주기 */
    if (ftruncate(dst_fd, src_size) < 0)
    {
        close(dst_fd);
        munmap(src, src_size);
        unix_error("ftruncate");
    }

    /* 복사될 파일 매핑*/
    if ((dst = (char *)mmap(NULL, src_size, PROT_READ | PROT_WRITE, MAP_SHARED, dst_fd, 0)) == MAP_FAILED)
    {
        close(dst_fd);
        munmap(src, src_size);
        unix_error("mmap");
    }

    /* 복사될 파일의 파일기술자 닫기 (매핑이 성공 했으므로 이제 파일기술자는 필요 없다.) */
    close(dst_fd);

    /* 파일 복사 */
    memcpy(dst, src, src_size);

    /* 매핑 해제 */
    munmap(dst, src_size);
    munmap(src, src_size);
    
    exit(EXIT_SUCCESS);
}

void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}