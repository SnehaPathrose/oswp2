//
// Created by Toby Babu on 11/4/17.
//

#include <sys/contextswitch.h>
#include <sys/allocator.h>
#include <sys/io.h>
#include <sys/gdt.h>
#include <sys/process.h>
#include <sys/tarfs.h>
#include <sys/pic.h>
#include <sys/idt.h>
#include <sys/klibc.h>

int count = 0;

void switch_to(struct PCB *me, struct PCB *next) {
    currentthread = next;
    uint64_t pml4 = (uint64_t) next->page_table & ~0xFFF;
    __asm__ volatile("mov %0, %%cr3"::"r"(pml4));
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

void switch_to_process(struct PCB *me, struct PCB *next) {
    currentthread = next;
    if (me!=NULL) {
        //__asm__ volatile("\t pop %r8\n");
        //__asm__ volatile("\t pop %r9\n");
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
void switch_to_new_process(struct PCB *tss) {
    set_tss_rsp(&tss->kstack[399]);
    uint64_t pml4 = (uint64_t)tss->page_table & ~0xFFF;
    __asm__ volatile("\t cli\n");
    __asm volatile("mov %0, %%cr3":: "r"(pml4));
    uint64_t new_rsp = tss->ursp - 32;
    uint64_t new_goto = *((uint64_t *)new_rsp);
    __asm__ volatile("\t movq %0,%%rax\n" :: "r"(new_rsp) );
    __asm__ volatile("\t pushq %rax\n" );
    __asm__ volatile("\t pushfq\n" );
    __asm__ volatile("\t popq %rax\n" );
    __asm__ volatile("\t orq $0x200,%rax\n" );
    __asm__ volatile("\t pushq %rax\n" );
    __asm__ volatile("\t pushq $0x2b\n" );
    __asm__ volatile("\t push %0\n" : "=m"(new_goto));
    __asm__ volatile("\t iretq\n" );
}
struct PCB* create_duplicate_process(struct PCB* forked_process) {
    struct PCB *duplicate_process = bump(sizeof(struct PCB));
    for (int i = 0; i < KSTACKLEN; i++) {
        duplicate_process->kstack[i] = forked_process->kstack[i];
    }
    duplicate_process->rsp = forked_process->rsp;
    duplicate_process->mm = forked_process->mm;
    duplicate_process->next = forked_process->next;
    duplicate_process->parent = forked_process->parent;
    duplicate_process->state = 1;
    duplicate_process->gotoaddr = forked_process->gotoaddr;
    struct PCB *temp = threadlist;
    while(temp->next != NULL)
        temp = temp->next;
    temp->next = duplicate_process;
    return duplicate_process;
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
    temp = threadlist;
    while(temp!=NULL)
    {
        if(temp->state == 0) {
            temp->state = 2;
            break;
        }
        temp=temp->next;

    }
    __asm__ volatile("\t sti\n" );
    schedule();
}

struct PCB *create_thread(uint64_t faddr)
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
    thread->page_table =  (struct pml4t *) ((uint64_t) pml4t_t - KERNBASE);
    kprintf("value of kstack %p",thread->rsp);
    struct PCB *temp = threadlist;
    while(temp->next != NULL)
        temp = temp->next;
    temp->next = thread;
    return thread;
}

char *check_for_ip(int size_ip, char *ab) {
    while(1) {
        int size = get_terminal_size();
        if (size == size_ip) {
            //return "";
            //kprintf("In check for ip");
            char *buf = get_terminal_buf();
            ab = kstrncopy(ab, buf, size_ip);
            reset_terminal();
            return ab;
        }
        //schedule();
    }
}

void fun1()
{
    on_completion_pointer = &thread_on_completion;
    void (*idle)() = &idle_func;
    create_thread((uint64_t)idle);
    create_thread((uint64_t)&user_process);
    schedule();
    /* char *ab = bump(10);;
     ab = check_for_ip(10, ab);
     kprintf("Outside ip %s", ab);*/
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
    if(next == NULL)
        next = mainthread;
    if(next != NULL)
        next->state = 0;
    switch_to(me,next);
}

void context_switch() {
    initialize_keyboard_buffer();
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
    __asm__ volatile("\t movq %0,%%rax\n" :: "r"(tss->ursp) );
    __asm__ volatile("\t pushq %rax\n" );
    __asm__ volatile("\t pushfq\n" );
    __asm__ volatile("\t popq %rax\n" );
    __asm__ volatile("\t orq $0x200,%rax\n" );
    __asm__ volatile("\t pushq %rax\n" );
    __asm__ volatile("\t pushq $0x2b\n" );
    __asm__ volatile("\t push %0\n" : "=m"(tss->gotoaddr) );
    __asm__ volatile("\t iretq\n" );
}