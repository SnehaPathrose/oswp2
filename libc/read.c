//
// Created by Toby Babu on 12/2/17.
//
#include <unistd.h>

ssize_t read(int fd, void *msg, int size)
{
    size_t ret;
    __asm__ volatile("movq $10,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}