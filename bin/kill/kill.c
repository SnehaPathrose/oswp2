//
// Created by Toby Babu on 12/5/17.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>

int main(int argc, char* argv[],char *envp[]) {
    if(argv[1][0] == '-') {
        int signal = atoi(argv[1]+1);
        pid_t pid = (pid_t) atoi(argv[2]);
        if(signal == 9) {
            kill(pid, signal);
        }
    }
    return 0;
}

