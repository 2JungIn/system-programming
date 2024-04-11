/**
 * redirection-example.c
 * 
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

#define DEFAULT_FLAG  (O_WRONLY | O_CREAT)
#define DEFAULT_MODE  (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)

void unix_error(const char *msg);   /* error handling function */

int io_redirection(const int argc, const char **argv)
{
    int fd;
    for (int i = 0; i < argc; i++)
    {
        if (!strcmp(argv[i], "<"))  /* stdin redirection */
        {
            argv[i] = NULL;

            if ((fd = open(argv[i + 1], O_RDONLY, DEFAULT_MODE)) < 0)    /* error: open */
                unix_error("open");
            
            if (dup2(fd, STDIN_FILENO) < 0)  /* error: dup2() */
                unix_error("dup2");
            
            if (close(fd) < 0) 
                unix_error("close");  /* error: close() */

            argv[i + 1] = NULL;
            i++;
            continue;
        }
        else if (!strcmp(argv[i], ">"))  /* stdout redirection (trunc) */
        {
            argv[i] = NULL;

            if ((fd = open(argv[i + 1], DEFAULT_FLAG | O_TRUNC, DEFAULT_MODE)) < 0)    /* error: open() */
                unix_error("open");

            if (dup2(fd, STDOUT_FILENO) < 0)  /* error: dup2() */
                unix_error("dup2");

            if (close(fd) < 0)   /* error: close() */
                unix_error("close");

            argv[i + 1] = NULL;
            i++;
            continue;
        }
        else if (!strcmp(argv[i], ">>"))  /* stdout redirection (append) */
        {
            argv[i] = NULL;

            if ((fd = open(argv[i + 1], DEFAULT_FLAG | O_APPEND, DEFAULT_MODE)) < 0)    /* error: open() */
                unix_error("open");

            if (dup2(fd, STDOUT_FILENO) < 0)  /* error: dup2() */
                unix_error("dup2");

            if (close(fd) < 0)   /* error: close() */
                unix_error("close");

            argv[i + 1] = NULL;
            i++;
            continue;
        }
    }

    return 0;
}

int main(void)
{
    // sort < input.txt > output.txt
    const int argc = 5;
    const char *argv[] = {"sort", "<", "input.txt", ">", "output.txt", NULL};
    
    pid_t child_pid;
    switch ((child_pid = fork()))
    {
        case -1:    /* error: fork() */
            unix_error("fork");
        case  0:
            if (io_redirection(argc, argv) < 0)    /* error: io_redirection() */
            {
                fprintf(stderr, "io_redirection() error!\n");
                exit(EXIT_FAILURE);
            }
            
            if (execvp(argv[0], (char *const *)argv) < 0)   /* error: execvp() */
                unix_error("execvp");

        default:
            if (waitpid(child_pid, NULL, 0) < 0) /* error: waitpid() */
                unix_error("waidpid");
    }

    exit(EXIT_SUCCESS);
}

void unix_error(const char *msg)    /* error handling function */
{
    perror(msg);
    exit(EXIT_FAILURE);
}