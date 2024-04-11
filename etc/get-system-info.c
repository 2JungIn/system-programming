#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

void unix_error(const char *msg);   /* error handling function */

int main(void) {
    struct utsname uts;

    if (uname(&uts) < 0) /* error: uname() */
        unix_error("uname");

    printf("OSname : %s\n", uts.sysname);
    printf("Nodename : %s\n", uts.nodename);
    printf("Release : %s\n", uts.release);
    printf("Version : %s\n", uts.version);
    printf("Machine : %s\n", uts.machine);

    exit(EXIT_SUCCESS);
}

void unix_error(const char *msg)    /* error handling function */
{
    perror(msg);
    exit(EXIT_FAILURE);
}