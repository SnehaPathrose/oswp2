//
// Created by Toby Babu on 11/4/17.
//

#include <sys/contextswitch.h>
#include <sys/allocator.h>
#include <sys/io.h>
#include <sys/gdt.h>
#include <sys/process.h>
#include <sys/tarfs.h>

int count = 0;

void switch_to(struct PCB *me, struct PCB *next) {
    if (me!=NULL) {
        __asm__ volatile("\t pop %r8\n");
        __asm__ volatile("\t pop %r9\n");
        __asm__ volatile("\t push %rax\n");
        __asm__ volatile("\t push %rbp\n");
        __asm__ volatile("\t push %rbx\n");
        __asm__ volatile("\t push %rcx\n");
        __asm__ volatile("\t push %rdx\n");
        __asm__ volatile("\t push %rdi\n");
        __asm__ volatile("\t push %rsi\n");
        __asm__ volatile("\t mov %%rsp,%0\n" : "=m"(me->rsp));
        __asm__ volatile("\t mov %0, %%rsp\n" : "=m"(next->rsp));
        __asm__ volatile("\t pop %rsi\n");
        __asm__ volatile("\t pop %rdi\n");
        __asm__ volatile("\t pop %rdx\n");
        __asm__ volatile("\t pop %rcx\n");
        __asm__ volatile("\t pop %rbx\n");
        __asm__ volatile("\t pop %rbp\n");
        __asm__ volatile("\t pop %rax\n");
        __asm__ volatile("\t ret\n" );
    }
    else
    {
        __asm__ volatile("\t mov %0, %%rsp\n" : "=m"(next->rsp));
        __asm__ volatile("\t pop %rsi\n");
        __asm__ volatile("\t pop %rdi\n");
        __asm__ volatile("\t pop %rdx\n");
        __asm__ volatile("\t pop %rcx\n");
        __asm__ volatile("\t pop %rbx\n");
        __asm__ volatile("\t pop %rbp\n");
        __asm__ volatile("\t pop %rax\n");
        __asm__ volatile("\t ret\n" );
    }
}

void idle_func()
{
    while(1) {
        schedule();
    };
}

void thread_on_completion()
{
    struct PCB *temp;
    temp=threadlist;
    while(temp!=NULL)
    {
        if(temp->state == 0) {
            temp->state = 2;
            break;
        }
        temp=temp->next;

    }
    schedule();
}

void create_thread(uint64_t faddr)
{
    struct PCB *thread = bump(sizeof(struct PCB));
    for (int i = 0; i < KSTACKLEN; i++) {
        thread->kstack[i] = 0;
    }
    void (*retfun)() = &thread_on_completion;
    thread->gotoaddr = faddr;
    thread->next = NULL;
    thread->state = 1;
    thread->kstack[KSTACKLEN - 1] = (uint64_t)retfun;
    thread->kstack[KSTACKLEN - 2] = faddr;
    thread->rsp=(uint64_t)&(thread->kstack[KSTACKLEN - 9]);
    struct PCB *temp = threadlist;
    while(temp->next != NULL)
        temp = temp->next;
    temp->next = thread;
}

void fun1()
{
    on_completion_pointer = &thread_on_completion;
    void (*idle)() = &idle_func;
    create_thread((uint64_t)idle);
    create_thread((uint64_t)&user_process);
    schedule();
}



void schedule() {
    struct PCB *me=NULL,*next=NULL,*temp;

    //some scheduling logic for now
    temp = threadlist;
    while(temp!=NULL) {
        if (temp->state == 0) {
            me = temp;
            break;
        }
        temp = temp->next;
    }
    temp=threadlist;
    while(temp!=NULL) {
        if (temp->state == 1) {
            next = temp;
            break;
        }
        temp=temp->next;

    }
    count++;

    if (me == mainthread)
        me->state = 2;
    else if(me != NULL)
        me->state = 1;
    if(next==NULL)
        next = mainthread;
    if(next!=NULL)
        next->state = 0;
    switch_to(me,next);
}

void context_switch() {
    void (*fun)()=&fun1;
    create_thread((uint64_t)fun);
    schedule();
}

void switch_to_ring_3(struct PCB *tss) {
    set_tss_rsp(&tss->kstack[399]);
    uint64_t pml4 = (uint64_t)tss->page_table & ~0xFFF;
    __asm__ volatile("\t cli\n");
    __asm volatile("mov %0, %%cr3":: "r"(pml4));
    __asm__ volatile("\t mov $0x23,%rax\n");
    __asm__ volatile("\t mov %rax,%ds\n");
    __asm__ volatile("\t mov %rax,%es\n");
    __asm__ volatile("\t mov %rax,%fs\n");
    __asm__ volatile("\t mov %rax,%gs\n");
    __asm__ volatile("\t pushq $0x23\n" );
    __asm__ volatile("\t movq %0,%%rax\n" :: "r"(tss->rsp) );
    __asm__ volatile("\t pushq %rax\n" );
    __asm__ volatile("\t pushfq\n" );
    __asm__ volatile("\t popq %rax\n" );
    __asm__ volatile("\t orq $0x200,%rax\n" );
    __asm__ volatile("\t pushq %rax\n" );
    __asm__ volatile("\t pushq $0x2b\n" );
    __asm__ volatile("\t push %0\n" : "=m"(tss->gotoaddr) );
    __asm__ volatile("\t iretq\n" );
}