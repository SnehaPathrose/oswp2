#include <stdio.h>
#include <unistd.h>

int putchar(int c)
{
    // write character to stdout
    char buf[2];
    buf[0] = (char) c;
    buf[1] = '\0';
    write(1,buf,25);

    return buf[0];
}
