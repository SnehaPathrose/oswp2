//
// Created by Toby Babu on 11/23/17.
//

int write(int fd, char *msg,int size)
{
    int ret;
    __asm__ volatile("movq $0,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}