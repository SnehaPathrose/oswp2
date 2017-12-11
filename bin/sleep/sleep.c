//
// Created by sneha pathrose on 12/5/17.
//
#include<unistd.h>
#include<string.h>
int main(int argc,char* argv[],char *envp[])
{
    unsigned int seconds = atoi(argv[1]);
    sleep(seconds);
}

