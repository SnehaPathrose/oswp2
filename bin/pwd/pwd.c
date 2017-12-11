//
// Created by sneha pathrose on 11/23/17.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

int main(int argc, char* argv[],char *envp[]) {
    //DIR *directory;
    //int c;
    char buf[20];
    //umemset(buf,0,20);
    //struct dirent *files/*= (struct dirent *)malloc(5*sizeof(struct dirent))*/;
    char *cwd;
    cwd=getcwd(buf,20);
    puts(cwd);
    return 0;
}
