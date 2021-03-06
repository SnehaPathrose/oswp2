#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/defs.h>

int open(const char *pathname, int flags);
int close(int fd);
ssize_t read(int fd, void *buf, int count);
int write(int fd, char *msg,int size);
int unlink(const char *pathname);

int chdir(const char *path);
char *getcwd(char *buf, size_t size);

int fork();
int execvpe(const char *file, char *const argv[], char *const envp[]);
pid_t wait(int *status);
int waitpid(int pid, int *status);
int access(const char *pathname, int mode);

unsigned int sleep(unsigned int seconds);

pid_t getpid(void);
pid_t getppid(void);

// OPTIONAL: implement for ``on-disk r/w file system (+10 pts)''
off_t lseek(int fd, off_t offset, int whence);
int mkdir(const char *pathname, uint64_t mode);

// OPTIONAL: implement for ``signals and pipes (+10 pts)''
int pipe(int pipefd[2]);
#define F_OK 1
#define R_OK 2
#define W_OK 3
#define X_OK 4

#endif


