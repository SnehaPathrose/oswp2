//
// Created by sneha pathrose on 12/7/17.
//
#include <unistd.h>
int main(int argc, char *argv[],char *envp[])
{
    char *param[]={"bin/sbush",NULL};
    char *env[]={"/rootfs/bin/",NULL};
    pid_t pid;
    int status;
    pid=fork();
    if(pid==0)
    {
        execvpe(param[0],param,env);
    }
    else
    {
        waitpid(pid,&status);
    }

}