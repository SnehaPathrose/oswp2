//
// Created by sneha pathrose on 12/5/17.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char* argv[])
{
    //int fileptr=0;
    char *filename=0;
    char *cwd;
    //ssize_t s;

    //cwd=getcwd(buf,50);
    //char *line=(char *)malloc(500);
    if(argv[1][0]!='/')
    {
        char *buf=(char *)malloc(25);
        cwd=getcwd(buf, 25);
        filename=strcat(cwd,argv[1]);

    }
    else
        filename=argv[1];

    chdir(filename);
    return 0;
}
