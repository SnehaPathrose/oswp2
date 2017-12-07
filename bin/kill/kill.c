//
// Created by Toby Babu on 12/5/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>

int main(int argc, char* argv[]) {
    if(argv[1][0] == '-') {
        int signal = atoi(argv[1]+1);
        pid_t pid = (pid_t) atoi(argv[2]);
        if(signal == 9) {
            kill(pid, signal);
            //puts("The process to be killed is: ");
            //puts(argv[2]);
        }
    }
    /*DIR *directory;
    int c;
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
    }*/
    return 0;
}


