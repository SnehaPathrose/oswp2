//
// Created by sneha pathrose on 11/23/17.
//
#include <stdlib.h>
void *malloc(size_t size)
{
    uint64_t ret;
    __asm__ volatile("movq $2,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return (void *)ret;
}
void exit(int status)
{
    __asm__ volatile("movq $9,%rax");
    __asm__ volatile("int $0x80");
}

void free(void *ptr)
{
    __asm__ volatile("movq $18,%rax");
    __asm__ volatile("int $0x80");
}