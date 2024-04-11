/**
 * multi-pipe-example.c
 * 
**/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ    0
#define WRITE   1

#define MAX_ARG 64

struct list_t
{
    char *argv[MAX_ARG];
    struct list_t *next;
};

void unix_error(const char *msg);    /* error handling function */

void multi_pipe(const struct list_t *cmd_list)
{
    if (cmd_list->next != NULL)  /* 파이프라인 처리 */
    {
        int fd[2];
        if (pipe(fd) < 0)    /* error: pipe() */
            unix_error("pipe");

        switch (fork())
        {
            case -1:    /* error: fork() */
                unix_error("fork");
            case  0:    /* child process */
                if (dup2(fd[READ], STDIN_FILENO) < 0)   /* error: dup2() */
                    unix_error("dup2");   

                if (close(fd[READ]) < 0)    /* error: close() */
                    unix_error("close");  

                if (close(fd[WRITE]) < 0)   /* error: close() */
                    unix_error("close");
                    
                    
                multi_pipe(cmd_list->next);
            default:    /* parent process */
                if (dup2(fd[WRITE], STDOUT_FILENO) < 0)     /* error: dup2() */
                    unix_error("dup2");

                if (close(fd[READ]) < 0)    /* error: close() */
                    unix_error("close");

                if (close(fd[WRITE]) < 0)   /* error: close() */
                    unix_error("close");

                if (execvp(cmd_list->argv[0], cmd_list->argv) < 0)    /* error: execvp() */
                    unix_error("execvp");
        }
    }

    if (execvp(cmd_list->argv[0], cmd_list->argv) < 0)    /* error: execvp() */
        unix_error("execvp");
}

int execute(const struct list_t *cmd_list)
{
    pid_t pid;
    switch ((pid = fork()))
    {
        case -1:    /* error: fork() */
            unix_error("fork");
        case  0:    /* child process */
            multi_pipe(cmd_list);
        default:    /* parent process */
            if (waitpid(pid, NULL, 0) < 0)    /* error: waitpid() */
                unix_error("waitpid");
    }
}

int main(void)
{
    // $ ls -l | sort | head -n 5
    struct list_t ls = { .argv={ "ls", "-l", NULL } };
    struct list_t sort = { .argv={ "sort", NULL } };
    struct list_t head = { .argv={ "head", "-n", "5", NULL } };
    
    ls.next = &sort;
    sort.next = &head;
    head.next = NULL;

    execute(&ls);

    exit(EXIT_SUCCESS);
}

void unix_error(const char *msg)    /* error handling function */
{
    perror(msg);
    exit(EXIT_FAILURE);
}