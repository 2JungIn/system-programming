/**
 * dir-traversal.c 
 * 
 * 디렉토리 순회를 구현한 프로그림 입니다.
**/

#define _DEFAULT_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>
#include <dirent.h>

#define MAX_FILE_CNT 256

void unix_error(const char *msg);
void swap(struct dirent **e1, struct dirent **e2);
int comp(const struct dirent *e1, const struct dirent *e2);
void print_tab(const int depth);

void directory_traversal(const char *path, unsigned int depth)
{
    struct dirent *entries[MAX_FILE_CNT];
    int file_cnt = 0;
    
    if (chdir(path) < 0)
        unix_error("chdir");

    DIR *dp = NULL;
    if ((dp = opendir(".")) == NULL)
        unix_error("opendir");

    /* scan file */
    while ((entries[file_cnt] = readdir(dp)))
    {
        if (!strcmp(entries[file_cnt]->d_name, "."))
            continue;
        else if (!strcmp(entries[file_cnt]->d_name, ".."))
            continue;
        else
            file_cnt++;
    }

    if (errno && entries[file_cnt] == NULL)
        unix_error("readdir");

    /* sorting */
    for (int i = 0; i < file_cnt - 1; i++)
    {
        int selected_index = i;
        for (int j = i + 1; j < file_cnt; j++)
        {
            if (comp(entries[j], entries[selected_index]))
                selected_index = j;
        }
        swap(&entries[i], &entries[selected_index]);
    }

    /* print */
    printf("[%s]\n", path);

    for (int i = 0; i < file_cnt; i++)
    {
        print_tab(depth);

        struct dirent *e = entries[i];
        if (e->d_type == DT_REG)
            printf("%s\n", e->d_name);
        else
            directory_traversal(e->d_name, depth + 1);
    }

    /* recovery */
    if (closedir(dp) < 0)
        unix_error("closedir");
    
    if (chdir("..") < 0)
        unix_error("chdir");
}


int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    directory_traversal(argv[1], 0);

    exit(EXIT_SUCCESS);
}


void unix_error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void swap(struct dirent **e1, struct dirent **e2)
{
    struct dirent *temp = *e1;
    *e1 = *e2;
    *e2 = temp;
}

int comp(const struct dirent *e1, const struct dirent *e2)
{
    if (e1->d_type == e2->d_type)
        return strcmp(e1->d_name, e2->d_name);

    return e1->d_type < e2->d_type;
}

void print_tab(const int depth)
{
    for (int i = 0; i <= depth; i++)
        printf("    ");
}
