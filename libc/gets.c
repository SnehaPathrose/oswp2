//
// Created by Toby Babu on 12/3/17.
//
#include <stdio.h>
#include <unistd.h>

char *gets(char *s) {
    /*char *temp = s;
    for( ; *temp != '\n'; ++temp) {
        char c = getchar();
        *temp = c;
        if (*temp == '\n'){
            return s;
        }
    }
    return "";*/
    //char buf[1];
    //buf[0] = '\0';
    read(0, s, 0);
    //putchar(buf[0]);
    return s;
}