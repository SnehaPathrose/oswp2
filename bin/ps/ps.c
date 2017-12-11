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
    struct dirent *files;
    directory=opendir("/proc/");
    if(directory>0)
    {
        write(1, "PID  PPID  STATUS     NAME\n", 27);
        while(1) {
            files = readdir(directory);
            if (files != NULL) {
                char pid[5], ppid[5];
                int i = 0, intvalue = files->pid;
                while(intvalue!=0)
                {
                    pid[i++] = intvalue % 10 + '0';
                    intvalue = intvalue / 10;
                }
                pid[i] = '\0';

                if (files->ppid == 0) {
                    ppid[0] = '0';
                    ppid[1] = '\0';
                }
                else {
                    i = 0, intvalue = files->ppid;
                    while(intvalue!=0)
                    {
                        ppid[i++] = intvalue % 10 + '0';
                        intvalue = intvalue / 10;
                    }
                    ppid[i] = '\0';
                }
                if(files->state == 2) {
                    continue;
                }
                write(1, pid, strlen(pid));
                write(1, "    ", 4);
                write(1, ppid, strlen(ppid));
                write(1, "     ", 5);
                if (files->state == 0)
                    write(1, "RUNNING ", 8);
                else if (files->state == 1)
                    write(1, "SLEEPING", 8);
                else if (files->state == 2)
                    write(1, "ZOMBIE  ", 8);
                write(1, "   ", 3);
                write(1, files->d_name, strlen(files->d_name));

                write(1, "\n", 1);
            }
            else
                break;
        }
        //write(1, "\n", 10);
        c=closedir(directory);
        if(c==-1)
            write(1,"Close failed\n",13);
    }
    return 0;
}




