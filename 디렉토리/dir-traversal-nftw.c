#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>    /* nftw() */


void unix_error(const char *msg);
void print_tab(const int cnt);


int display_info(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf)
{
    print_tab(ftwbuf->level);

    if (tflag == FTW_D)
        printf("[%s]\n", ftwbuf->base + fpath);
    else
        printf("%s\n", ftwbuf->base + fpath);

    return 0;           /* To tell nftw() to continue */
}


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (nftw((argc < 2) ? "." : argv[1], display_info, 20, FTW_PHYS) == -1)
        unix_error("nftw");

    exit(EXIT_SUCCESS);
}


void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void print_tab(const int cnt)
{
    for (int i = 0; i < cnt; i++)
        printf("    ");
}