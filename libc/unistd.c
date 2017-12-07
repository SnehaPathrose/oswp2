//
// Created by sneha pathrose on 11/24/17.
//
#include <unistd.h>
char *getcwd(char *buf, size_t size)
{
    uint64_t ret;
    __asm__ volatile("movq $3,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return (char *)ret;
}
int execvpe(const char *file, char *const argv[])
{
    int ret;
    __asm__ volatile("movq $7,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}

int fork()
{
    int ret;
    __asm__ volatile("movq $8,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}

int waitpid(int pid, int *status) {
    int ret;
    __asm__ volatile("movq $11,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}

int access(const char *pathname, int mode) {
    int ret;
    __asm__ volatile("movq $12,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}

int open(const char *pathname, int flags){
    int ret;
    __asm__ volatile("movq $13,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}

int close(int fd)
{
    int ret;
    __asm__ volatile("movq $14,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}
unsigned int sleep(unsigned int seconds)
{
    unsigned int ret;
    __asm__ volatile("movq $15,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}

