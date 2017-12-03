//
// Created by Toby Babu on 12/3/17.
//
#include <unistd.h>
#include <stdio.h>

int getchar() {
    char buf[1];
    buf[0] = '\0';
    read(1,buf, 10);
    //putchar(buf[0]);
    return buf[0];
}