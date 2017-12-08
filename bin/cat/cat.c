//
// Created by sneha pathrose on 12/5/17.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char* argv[])
{
    int fileptr=0;
    char *filename=0;
    char *cwd;
    ssize_t s;

    //cwd=getcwd(buf,50);
    char *line=(char *)malloc(100);
    if(argv[1][0]!='/')
    {
        char *buf=(char *)malloc(25);
        cwd=getcwd(buf,25);
        filename=strcat(cwd,argv[1]);

    }
    else
        filename=argv[1];

    fileptr=open(filename,0);
    if(fileptr<0)
    {
        puts("File Not Found\n");
        return 1;
    }
    umemset(line, 0, 100);
    s=read(fileptr,line,100);
    write(1,line,s);
    while(s != EOF) {
        umemset(line, 0, strlen(line));
        s = read(fileptr, line, 100);
        if (s == EOF)
            break;
        write(1,line,s);
    }
    close(fileptr);
    return 0;
}

