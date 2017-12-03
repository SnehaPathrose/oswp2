//
// Created by Toby Babu on 11/24/17.
//

int fork() {
    int ret;
    __asm__ volatile("movq $3,%rax");
    __asm__ volatile("int $0x80":"=a"(ret));
    return ret;
}