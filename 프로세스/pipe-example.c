/**
 * pipe-example.c
 * 
**/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define READ    0
#define WRITE   1

void unix_error(const char *msg);   /* error handling function */

int main(void)
{
    // $ ls | head -n 5
    const char *ls[] = {"ls", "-l", NULL};
    const char *head[] = {"head", "-n", "5", NULL};

    int fd[2];
    pid_t pid;
    if (pipe(fd) < 0)   /* error: pipe() */
        unix_error("pipe");

    switch ((pid = fork()))
    {
        case -1:    /* error: fork() */
            unix_error("fork");
        case  0:    /* child process */
            if (dup2(fd[READ], STDIN_FILENO) < 0)  /* error: dup2() */
                unix_error("dup2"); 

            if (close(fd[READ]) < 0)    /* error: close() */
                unix_error("close"); 

            if (close(fd[WRITE]) < 0)    /* error: close() */
                unix_error("close"); 

            if (execvp(head[0], (char *const *)head) < 0)   /* error: execvp() */
                unix_error("execvp"); 
        default:    /* parent process */
            if (dup2(fd[WRITE], STDOUT_FILENO) < 0)  /* error: dup2() */
                unix_error("dup2"); 
            
            if (close(fd[READ]) < 0)    /* error: close()*/
                unix_error("close"); 

            if (close(fd[WRITE]) < 0)    /* error: close() */
                unix_error("close"); 

            if (execvp(ls[0], (char *const *)ls) < 0)    /* error: execvp() */
                unix_error("execvp");   
    }

    exit(EXIT_SUCCESS);
}

void unix_error(const char *msg)    /* error handling function */
{
    perror(msg);
    exit(EXIT_FAILURE);
}