/**
 * fork-exec-example.c
 * 
**/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void unix_error(const char *msg);    /* error handling function */


int main(int argc, char *argv[])
{
    const char *ls[] = { "ls", NULL };

    pid_t child_pid;
    switch (child_pid = fork())
    {
        case -1: /* error: fork() */
            unix_error("fork");
        case  0: /* child process */
            if (execvp(ls[0], (char *const *)ls) < 0)    /* error: execvp() */
                unix_error("execvp");
        default: /* parent process */
            if (waitpid(child_pid, NULL, 0) < 0)     /* error: waitpid() */
                unix_error("waitpid");
    }

    exit(EXIT_SUCCESS);
}

void unix_error(const char *msg)    /* error handling function */
{
    perror(msg);
    exit(EXIT_FAILURE);
}