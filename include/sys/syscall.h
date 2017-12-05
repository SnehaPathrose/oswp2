//
// Created by Toby Babu on 11/22/17.
//

#ifndef COURSEPROJ_SYSCALL_H
#define COURSEPROJ_SYSCALL_H
#include <sys/defs.h>
#include <sys/fs.h>
#define MAXSYSCALLS 13

uint64_t sys_getpid();
int sys_write(int fd,char *msg,int size);
uint64_t sys_malloc(uint64_t size);
char *sys_getcwd(char *buf, size_t size);
DIR *sys_opendir(char *dirname);
struct dirent *sys_readdir(DIR *dirp);
int sys_closedir(DIR *dirp);
int sys_execvpe(const char *file, char *const argv[]);
void syscall_handler();
int sys_fork();
int sys_read(int fd,char *msg,int size);
void sys_exit(int status);
int sys_waitpid(int pid, int *status);
int sys_access(const char *pathname, int mode);

#endif //COURSEPROJ_SYSCALL_H

