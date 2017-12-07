//
// Created by Toby Babu on 12/5/17.
//

#include <signal.h>
int kill(pid_t pid, int sig)
{
    int ret;
    __asm__ volatile("movq $16,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}