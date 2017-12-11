#include <stdlib.h>

void _start() {
// call main() and exit() here
    uint64_t ret;
    __asm__ volatile(
    "mov 8(%rsp), %rdi\n\t");
    __asm__ volatile("add $16, %rsp\n\t");
    __asm__ volatile("movq (%rsp), %rsi\n\t");
    __asm__ volatile("add $8, %rsp\n\t");
    __asm__ volatile("movq (%rsp), %rdx\n\t");
    __asm__ volatile("call main\n\t":"=a"(ret)::"rdi");
    exit(0);
}