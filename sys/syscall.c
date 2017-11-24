//
// Created by sneha pathrose on 11/17/17.
//
#include <sys/syscall.h>
#include <sys/io.h>
const void  *syscalls[MAXSYSCALLS] = {&sys_write, &sys_getpid};
int sys_getpid(int num)
{
    //kprintf(msg);
    kprintf("number is %d ", num);
    return 2;
}
int sys_write(int fd,char *msg,int size)
{
    kprintf(msg);
    return 1;
}




void syscall_handler() {
    uint64_t syscallno;
    uint64_t ret;
    void (*sysaddress)();
    __asm volatile("movq 56(%%rsp), %0" : "=r" (syscallno));
    if (syscallno < MAXSYSCALLS) {
        sysaddress = (void *)syscalls[syscallno];
        __asm__ volatile ( "\tpushq %rdi\n");
        __asm__ volatile ( "\tpushq %rcx\n");
        __asm__ volatile ( "\tpushq %rdx\n");
        __asm__ volatile ( "\tpushq %rsi\n");
        __asm__ volatile ( "\tpushq %rbx\n");
        __asm__ volatile ( "callq *%0;":"=a"(ret) :"r" (sysaddress): "rsi","rdi","rbx","rcx","rdx");
        __asm volatile("movq %0, 104(%%rsp)" : "=r" (ret));
        __asm__ volatile ( "popq %rbx ");
        __asm__ volatile ( "popq %rsi ");
        __asm__ volatile ( "popq %rdx ");
        __asm__ volatile ( "popq %rcx ");
        __asm__ volatile ( "popq %rdi ");
    }
}