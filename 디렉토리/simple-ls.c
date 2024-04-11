/**
 * simple-ls.c
 * 
 * 디렉토리 api를 활용하여 ls명령어를 구현한 프로그램 입니다.
**/

#define _XOPEN_SOURCE 700

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
#include <dirent.h>

#define BUF_SIZE 50

void unix_error(const char *msg);

void print_time(const time_t *timer);
void print_fmt(const mode_t mode);
void print_permission(const mode_t mode);


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    DIR *dp = NULL;
    if ((dp = opendir(argv[1])) == NULL)
        unix_error("opendir");
    
    char buffer[BUFSIZ];
    struct dirent *e = NULL;
    struct stat s;
    for (e = readdir(dp); e; e = readdir(dp))
    {
        memset(&buffer, 0, sizeof(buffer));

        strcat(buffer, argv[1]);
        strcat(buffer, "/");
        strcat(buffer, e->d_name);

        if (lstat(buffer, &s) < 0)
            unix_error("lstat");

        /* print inode number */
        printf("%10lu ", s.st_ino);

        /* print format */
        print_fmt(s.st_mode);

        /* print permission */
        print_permission(s.st_mode);

        /* print owner uid */
        printf("%5u ", s.st_uid);
        
        /* print owner gid */
        printf("%5u ", s.st_gid);
        

        /* print file size */
        printf("%10ld ", s.st_size);

        /* print last file modification */
        print_time(&s.st_mtime);

        /* print file name */
        printf("%s\n", e->d_name);
    }

    if (errno != 0 && e == NULL)
        unix_error("readdir");

    if (closedir(dp) < 0)
        unix_error("closedir");

    exit(EXIT_SUCCESS);
}


void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_fmt(const mode_t mode)
{
    switch (mode & S_IFMT)
    {
    case S_IFBLK:  printf("    [block device]"); break;
    case S_IFCHR:  printf("[character device]"); break;
    case S_IFDIR:  printf("       [directory]"); break;
    case S_IFIFO:  printf("       [FIFO/pipe]"); break;
    case S_IFLNK:  printf("         [symlink]"); break;
    case S_IFREG:  printf("    [regular file]"); break;
    case S_IFSOCK: printf("          [socket]"); break;
    default:       printf("         [unknown]"); break;
    }

    printf(" ");
}

void print_permission(const mode_t mode)
{
    printf(mode & S_IRUSR ? "r" : "-");
    printf(mode & S_IWUSR ? "w" : "-");
    printf(mode & S_IXUSR ? "x" : "-");
    printf(mode & S_IRGRP ? "r" : "-");
    printf(mode & S_IWGRP ? "w" : "-");
    printf(mode & S_IXGRP ? "x" : "-");
    printf(mode & S_IROTH ? "r" : "-");
    printf(mode & S_IWOTH ? "w" : "-");
    printf(mode & S_IXOTH ? "x" : "-");

    printf(" ");
}

void print_time(const time_t *timer)
{
    char buffer[BUF_SIZE];

    struct tm *tmp = NULL;
    if ((tmp = localtime(timer)) == NULL) /* error: localtime() */
    {
        perror("localtime");
        return ;
    }
    
    if (strftime(buffer, sizeof(buffer), "%Y. %m. %d. (%a) %T %Z(GMT%z)", tmp) == 0)  /* error: strftime() */
    {
        fprintf(stderr, "strftime() returned 0!\n");
        return ;
    }

    printf("%s ", buffer);
}
