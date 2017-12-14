//
// Created by Toby Babu on 12/14/17.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char* argv[], char *envp[])
{
    //int fileptr=0;
    char *filename=0;
    //int len=0;
    char concatstr[25];
    char *cwd;
    char *buf=(char *)malloc(25);
    umemset(buf,'\0',25);
    //len=strlen(argv[1]);
    /*if(argv[1][len-1]!='/') {
        argv[1][len] = '/';
        argv[1][len+1]='\0';
    }*/
    //ssize_t s;

    //cwd=getcwd(buf,50);
    //char *line=(char *)malloc(500);
    if(argv[1][0]!='/')
    {

        cwd=getcwd(buf, 25);
        filename=strcat(cwd,argv[1],concatstr);

    }
    else
        filename=argv[1];

    unlink(filename);
    free(buf);
    return 0;
}



