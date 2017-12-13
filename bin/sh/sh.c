//
// Created by sneha pathrose on 12/10/17.
//
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#define BIN "bin/"
void bg_fg_process(char **command)
{
    int status,commandc;
    char *env[]={"/rootfs/bin/",NULL};
    char *filename = 0;
    char concatstr[20];
    pid_t pid;
    int a;
    commandc = tokencount(command);
    //commandc=1;
    //filename = concat(BIN,command[0]);
    //if(access(filename,F_OK)< 0)
    //filename=concat("/usr/",filename);

    pid = fork();
    if(pid == 0)
    {
        /*char *path_values;
        path_values = getenv("PATH");
        char **tokenized_path = tokenize_path(path_values);
        for (int i = 0; tokenized_path[i] != 0x0; i++) {
            filename = strcat(tokenized_path[i], command[0], concatstr);
            a = access(filename, F_OK);
            if (a != -1)
                break;
        }*/
        filename = strcat(BIN,command[0],concatstr);
        a=access(filename,F_OK);
        if(a==-1)
        {
            puts("Command not found");
            puts(filename);
            exit(0);

        }
        else {
            execvpe(filename,(char * const *)command,(char * const *)env);
        }



    }

    else if(pid > 0)
    {
        if(strcmp("&", command[commandc-1]) != 0)
        {
            waitpid(pid,&status);
        }
        else
        {
            return;
        }
    }
    //free(filename);
}

int main(int argc, char *argv[],char *envp[])
{
    int fileptr;
    char string3[25];
    //char token[5][25];
    // char *t[5]={token[0],token[1],token[2],token[3],token[4]};

    char *filename=NULL;
    // pid_t pid;
    char *line,**split;
    // int status;
    char *cwd;
    char *buf=(char *)malloc(25 * sizeof(char));
    umemset(buf,'\0',25);

    //puts(command[0]);


    if(argv[1][0]=='.')
    {

        cwd=getcwd(buf,25);
        filename=strcat(cwd,argv[1]+2,string3);
    }
    else
    if(argv[1][0]!='/')
    {
        cwd=getcwd(buf,25);
        filename=strcat(cwd,argv[1],string3);
    }

    else if(argv[1][0]=='/')
        filename=argv[1];

    fileptr = open(filename, 0);
    if(fileptr < 0)
    {
        /*int i = 0;
        char** abc = tokenizepath(getenv("PATH"));
        while(abc[i] != '\0') {
            abc[i] = concat(abc[i], "/");
            abc[i] = concat(abc[i], command[1]);
            filename = fopen(abc[i], O_RDONLY);
            if (filename > 0)
                break;

            i++;
        }
        if (filename < 0) {*/
        puts("File Not Found\n");
        return 1;
        /*}*/

    }
    line = (char *)malloc(50*sizeof(char));
    umemset(line,'\0',50);
    line = fgets(fileptr, line, 50);
    /*int fileptr = 0;
    fileptr = strlen(line) + 1;*/
    if(strcmp(line,"#!/bin/sbush")!=0)
    {
        puts("Script cannot be executed in this shell\n");
        return 1;
    }

    while(*line != EOF)
    {
        umemset(line, 0, strlen(line));

        line=fgets(fileptr, line, 50);
        // fileptr += strlength(line) + 1;
        if (line[0] == EOF)
            break;
        if(line[0]=='#')
            continue;
        split=tokenize(line);
        //umemset((void *)t,0,5);
        //pid=fork();
        // if(pid==0)
        //{
        bg_fg_process(split);
        // write(1,"hey",5);
        //  exit(0);

        /*}
        else
        if(pid>0)
        {
            waitpid(pid,&status);
        }*/
    }
    close(fileptr);
    free(buf);
    free(line);
    return 0;
}


