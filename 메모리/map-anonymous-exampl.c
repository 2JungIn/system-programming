/**
 * map-anonymous-exampl.c
 * 
 * 메모리 매핑 함수인 mmap()을 활용하여 익명 메모리 매핑을 하는 프로그램 입니다.<br>
 * (매핑된 메모리는 0으로 초기화 되어있습니다.)
**/

#define _GNU_SOURCE /* MAP_ANONYMOUS */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#include <fcntl.h>
#include <unistd.h>

void unix_error(const char *msg);
void print_memory(const char *memory, const size_t memory_size);

int main(void)
{
    size_t memory_size = 100;
    char *memory = NULL;

    /* MAP_ANONYMOUS 또는 MAP_ANON을 지원하는 경우 */
    // if ((memory = (char *)mmap(NULL, memory_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)) == MAP_FAILED)
    //     unix_error("mmap");

    /* MAP_ANONYMOUS 또는 MAP_ANON을 지원하지 않는 경우 */
    // int fd;
    // if ((fd = open("/dev/zero", O_RDWR)) < 0)
    //     unix_error("open");

    // if ((memory = (char *)mmap(NULL, memory_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
    //     unix_error("mmap");

    // if (close(fd) < 0)
    //     unix_error("close");

    print_memory(memory, memory_size);

    strcpy(memory, "Hello, World!");
    printf("%s\n", memory);

    print_memory(memory, memory_size);

    if (munmap(memory, memory_size) < 0)
        unix_error("munmap");
    
    exit(EXIT_SUCCESS);
}


void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_memory(const char *memory, const size_t memory_size)
{
    for (int i = 0; i < memory_size; i++)
    {
        printf("%02x", memory[i]);
        
        if ((i + 1) % 16 == 0)
            printf("\n");
        else if ((i + 1) % 8 == 0)
            printf("    ");
        else
            printf(" ");
    }

    printf("\n");
}
