//
// Created by sneha pathrose on 12/7/17.
//
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[],char *envp[])
{
    char *param[]={"bin/sbush",NULL};

    pid_t pid;
    int status;
    pid=fork();
    if(pid==0)
    {
        char *env[]={"bin/","sbush> ", NULL};
        execvpe(param[0], param, env);
    }
    else
    {
        waitpid(pid,&status);
    }

}
