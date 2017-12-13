//
// Created by Toby Babu on 11/22/17.
//

#ifndef COURSEPROJ_SYSCALL_H
#define COURSEPROJ_SYSCALL_H
#include <sys/defs.h>
#include <sys/fs.h>
#define MAXSYSCALLS 50

pid_t sys_getpid();
pid_t sys_getppid();
int sys_write(int fd,char *msg,int size);
uint64_t sys_malloc(uint64_t size);
void sys_free(void *ptr);
char *sys_getcwd(char *buf, size_t size);
DIR *sys_opendir(char *dirname);
struct dirent *sys_readdir(DIR *dirp);
int sys_closedir(DIR *dirp);
int sys_execvpe(const char *file, char *const argv[],char *const envp[]);
void syscall_handler(void *sysaddress);
int sys_fork();
int64_t sys_read(int fd,char *msg,int size);
void sys_exit(int status);
uint32_t sys_waitpid(uint32_t pid, int *status);
int sys_access(const char *pathname, int mode);
int sys_kill(uint32_t pid, int sig);
int sys_open(const char *pathname, int flags);
int sys_close(int fd);
unsigned int sys_sleep(unsigned int seconds);
int sys_chdir(const char *path);
char *sys_getenv(char *name);
int sys_setenv(char *name, char *value);
void initialise_syscalls();

//const void  *syscalls[MAXSYSCALLS];
#endif //COURSEPROJ_SYSCALL_H




