#include <stdio.h>
#include <time.h>

#define BUF_SIZE    50

char *date(void)
{
    static char buffer[BUF_SIZE];

    time_t now = time(NULL);
    struct tm *tmp = NULL;
    if ((tmp = localtime(&now)) == NULL) /* error: localtime() */
    {
        perror("localtime");
        return NULL;
    }
    
    if (strftime(buffer, sizeof(buffer), "%Y. %m. %d. (%a) %T %Z(GMT%z)", tmp) == 0)  /* error: strftime() */
    {
        fprintf(stderr, "strftime() returned 0!\n");
        return NULL;
    }

    return buffer;
}

int main(void)
{
    printf("%s\n", date());

    return 0;
}