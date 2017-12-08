//
// Created by sneha pathrose on 12/7/17.
//
#include <unistd.h>
int main(int argc, char *argv[])
{
    char *param[]={"bin/sbush",NULL};
    pid_t pid;
    int status;
    pid=fork();
    if(pid==0)
    {
        execvpe(param[0],param);
    }
    else
    {
        waitpid(pid,&status);
    }

}