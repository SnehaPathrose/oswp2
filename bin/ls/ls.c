//
// Created by sneha pathrose on 11/23/17.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char* argv[],char *envp[]) {
    DIR *directory;
    int c;
    /*for(c=0;c<argc;c++)
        write(1, argv[c], 10);
    write(1,"env",10);*/
    //char *buf=(char *)malloc(20);
    char buf[20];
    umemset(buf,0,20);
    /*for (int i = 0; i < 50; i++) {
        buf[i] = '\0';
    }*/
    struct dirent *files;
    char *cwd;
    cwd=getcwd(buf,20);
    //write(1,cwd,100);
    directory = opendir(cwd);
    if(directory>0)
    {
        while(1) {
            //umemset(files, 0, 5*sizeof(struct dirent));
            files = readdir(directory);
            if (files != NULL) {
                write(1, files->d_name, strlen(files->d_name));
                write(1, "  ", 2);
            }
            else
                break;
        }
        write(1, "\n", 1);
        c=closedir(directory);
        if(c==-1)
            write(1,"Close failed\n",13);
    }
    return 0;
}






