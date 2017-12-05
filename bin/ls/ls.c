//
// Created by sneha pathrose on 11/23/17.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
int main(int argc, char* argv[]) {
    DIR *directory;
    int c;
    /*for(c=0;c<argc;c++)
        write(1, argv[c], 10);
    write(1,"env",10);*/
    char *buf=(char *)malloc(50);
    struct dirent *files= (struct dirent *)malloc(5*sizeof(struct dirent));
    char *cwd;
    cwd=getcwd(buf,100);
    //write(1,cwd,100);
    directory=opendir(cwd);
    if(directory>0)
    {
        while(1) {
            files = readdir(directory);
            if (files != NULL) {
                write(1, files->d_name, 50);
                write(1, "\n", 10);
            }
            else
                break;
        }
        //write(1, "\n", 10);
        c=closedir(directory);
        if(c==-1)
            write(1,"Close failed\n",50);
    }
    return 0;
}


