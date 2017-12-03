//
// Created by Toby Babu on 12/3/17.
//
#include <stdio.h>
char *gets(char *s) {
    char *temp = s;
    for( ; *temp != '\n'; ++temp) {
        char c = getchar();
        *temp = c;
        if (*temp == '\n'){
            return s;
        }
    }
    return "";
}