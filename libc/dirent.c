//
// Created by sneha pathrose on 11/24/17.
//
#include <dirent.h>
#include <sys/defs.h>
DIR *opendir(const char *name)
{
    uint64_t ret;
    __asm__ volatile("movq $4,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return (DIR *)ret;
}

struct dirent *readdir(DIR *dirp)
{
    uint64_t ret;
    __asm__ volatile("movq $5,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return (struct dirent *)ret;
}

int closedir(DIR *dirp)
{
    int ret;
    __asm__ volatile("movq $6,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}

