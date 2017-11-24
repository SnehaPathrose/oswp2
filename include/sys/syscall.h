//
// Created by Toby Babu on 11/22/17.
//

#ifndef COURSEPROJ_SYSCALL_H
#define COURSEPROJ_SYSCALL_H
#include <sys/defs.h>
#define MAXSYSCALLS 2

int sys_getpid(int num);
int sys_write(int fd,char *msg,int size);
void syscall_handler();

#endif //COURSEPROJ_SYSCALL_H
