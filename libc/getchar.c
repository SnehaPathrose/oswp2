//
// Created by Toby Babu on 12/3/17.
//
#include <unistd.h>
#include <stdio.h>

int getchar() {
    char buf[1];
    buf[0] = '\0';
    read(0,buf, 1);
    //putchar(buf[0]);
    return buf[0];
}