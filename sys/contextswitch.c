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


void switch_to(struct PCB *me, struct PCB *next) {
    //currentthread = next;
    uint64_t pml4 = (uint64_t) next->page_table & ~0xFFF;
    __asm__ volatile("mov %0, %%cr3"::"r"(pml4));
    set_tss_rsp(&next->kstack[KSTACKLEN - 1]);
    __asm__ volatile("\t pop %r8\n");
    __asm__ volatile("\t pop %r8\n");
    __asm__ volatile("\t pop %r8\n");
    if (me != NULL) {
        __asm__ volatile("\t pop %r8\n");
        __asm__ volatile("\t pop %r9\n");
        __asm__ volatile("\t push %rax\n");
        __asm__ volatile("\t push %0\n" : "=m"(me->rbp));
        __asm__ volatile("\t push %0\n" : "=m"(me->rbx));
        __asm__ volatile("\t push %0\n" : "=m"(me->rcx));
        __asm__ volatile("\t push %0\n" : "=m"(me->rdx));
        __asm__ volatile("\t push %0\n" : "=m"(me->rdi));
        __asm__ volatile("\t push %0\n" : "=m"(me->rsi));
        //__asm__ volatile("\t push %r12\n");
        __asm__ volatile("\t mov %%rsp,%0\n" : "=m"(me->rsp));
        __asm__ volatile("\t mov %0, %%rsp\n" : "=m"(next->rsp));
        //__asm__ volatile("\t pop %r12\n");
        __asm__ volatile("\t pop %rsi\n");
        __asm__ volatile("\t pop %rdi\n");
        __asm__ volatile("\t pop %rdx\n");
        __asm__ volatile("\t pop %rcx\n");
        __asm__ volatile("\t pop %rbx\n");
        __asm__ volatile("\t pop %rbp\n");
        __asm__ volatile("\t pop %rax\n");
        __asm__ volatile("\t ret\n" );
    } else {
        __asm__ volatile("\t mov %0, %%rsp\n" : "=m"(next->rsp));
        // __asm__ volatile("\t pop %r12\n");
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


void idle_func() {
    while (1) {
        schedule();
    };
}

void thread_on_completion() {
    struct PCB *temp = NULL, *temp_list = blockedlist, *temp2 = NULL, *another_child = NULL, *next = NULL, *parent = NULL, *childnext = NULL/*, *prev_parent*/;
    temp = threadlist;
    while (temp != NULL) {
        if (temp->state == 0) {
            temp->state = 2;
            break;
        }

        temp = temp->next;
        next = temp->next;

    }

    if (temp != NULL && temp->ppid > 0 && temp->is_wait == 1) {
        temp2 = threadlist;
        while (temp2 != NULL) {
            if ((temp2->state != 2) && (temp != temp2) && (temp->ppid == temp2->ppid)) {
                another_child = temp2;
                childnext = another_child->next;
                break;
            }
            temp2 = temp2->next;
        }

        if (!another_child) {
            while (temp_list != NULL) {
                if (temp_list->pid == temp->ppid) {
                    parent = temp_list;
                    temp_list = temp_list->next;
                    blockedlist = temp_list;
                    //if (next != NULL) {
                    temp->next = parent;
                    parent->next = next;
                    //}
                    break;
                }
                temp_list = temp_list->next;
            }
        } else if (another_child != NULL && another_child->is_wait != 1) {
            while (temp_list != NULL) {
                if (temp_list->pid == another_child->ppid) {
                    parent = temp_list;
                    temp_list = temp_list->next;
                    blockedlist = temp_list;
                    //if (next != NULL) {
                    another_child->next = parent;
                    parent->next = childnext;
                    //}
                    break;
                }
                temp_list = temp_list->next;
            }
        }
    }
    __asm__ volatile("\t sti\n" );
    schedule();
}

struct PCB *create_thread(uint64_t faddr) {
    struct PCB *thread = bump(sizeof(struct PCB));
    for (int i = 0; i < KSTACKLEN; i++) {
        thread->kstack[i] = 0;
    }
    void (*retfun)() = &thread_on_completion;
    thread->gotoaddr = faddr;
    thread->next = NULL;
    thread->state = 1;
    thread->kstack[KSTACKLEN - 1] = (uint64_t) retfun;
    thread->kstack[KSTACKLEN - 2] = faddr;
    thread->rsp = (uint64_t) &(thread->kstack[KSTACKLEN - 9]);
    thread->page_table = (struct pml4t *) ((uint64_t) pml4t_t - KERNBASE);
    struct PCB *temp = threadlist;
    while (temp->next != NULL)
        temp = temp->next;
    temp->next = thread;
    return thread;
}

void fun1() {
    on_completion_pointer = &thread_on_completion;
    void (*idle)() = &idle_func;
    create_thread((uint64_t) idle);
    create_thread((uint64_t) &user_process);
    schedule();
}


void schedule() {
    struct PCB *me = NULL, *next = NULL, *temp;

    __asm__ volatile("mov %rdi,%r8");
    __asm__ volatile("mov %%r8,%0":"=m"(currentthread->rdi));
    __asm__ volatile("mov %%rbx,%0":"=r"(currentthread->rbx));
    __asm__ volatile("mov %%rcx,%0":"=r"(currentthread->rcx));
    __asm__ volatile("mov %%rbp,%0":"=r"(currentthread->rbp));
    __asm__ volatile("mov %%rsi,%0":"=r"(currentthread->rsi));
    //__asm__ volatile("mov %%r12,%0":"=r"(currentthread->r12));
    __asm__ volatile("mov %%rdx,%0":"=r"(currentthread->rdx));
    me = currentthread;
    temp = threadlist;
    if (me != NULL && me->next != NULL && me->state != 3) {
        temp = me->next;
        while (temp != NULL) {
            if (temp->state == 1) {
                next = temp;
                break;
            }
            temp = temp->next;

        }
        if (next == NULL) {
            temp = threadlist;
            while (temp != NULL) {
                if (temp->state == 1) {
                    next = temp;
                    break;
                }
                temp = temp->next;

            }
        }
    }
    else {
        while (temp != NULL) {
            if (temp->state == 1) {
                next = temp;
                break;
            }
            temp = temp->next;

        }
    }


    if (me == mainthread)
        me->state = 2;
    else if (me != NULL && me->state != 2)
        me->state = 1;
    if (next == NULL)
        next = mainthread;
    if (next != NULL)
        next->state = 0;
    currentthread = next;
    switch_to(me, next);

    /*struct PCB *me = NULL, *next = NULL, *temp;/
    __asm__ volatile("mov %rdi,%r8");
    __asm__ volatile("mov %%r8,%0":"=m"(currentthread->rdi));
    __asm__ volatile("mov %%rbx,%0":"=r"(currentthread->rbx));
    __asm__ volatile("mov %%rcx,%0":"=r"(currentthread->rcx));
    __asm__ volatile("mov %%rbp,%0":"=r"(currentthread->rbp));
    __asm__ volatile("mov %%rsi,%0":"=r"(currentthread->rsi));
    __asm__ volatile("mov %%rdx,%0":"=r"(currentthread->rdx));
    me = currentthread;
    temp = threadlist;
    while (temp != NULL) {
        if (temp->state == 1) {
            next = temp;
            break;
        }
        temp = temp->next;

    }


    if (me == mainthread)
        me->state = 2;
    else if (me != NULL && me->state != 2)
        me->state = 1;
    if (next == NULL)
        next = mainthread;
    if (next != NULL)
        next->state = 0;
    currentthread = next;
    switch_to(me, next);*/
}

void context_switch() {
    initialize_keyboard_buffer();
    void (*fun)()=&fun1;
    create_thread((uint64_t) fun);
    schedule();
}

void switch_to_ring_3(struct PCB *tss) {
    set_tss_rsp(&tss->kstack[399]);
    uint64_t pml4 = (uint64_t) tss->page_table & ~0xFFF;
    __asm__ volatile("\t cli\n");
    __asm volatile("mov %0, %%cr3"::"r"(pml4));
    __asm__ volatile("\t mov $0x23,%rax\n");
    __asm__ volatile("\t mov %rax,%ds\n");
    __asm__ volatile("\t mov %rax,%es\n");
    __asm__ volatile("\t mov %rax,%fs\n");
    __asm__ volatile("\t mov %rax,%gs\n");
    __asm__ volatile("\t pushq $0x23\n" );
    __asm__ volatile("\t movq %0,%%rax\n"::"r"(tss->ursp));
    __asm__ volatile("\t pushq %rax\n" );
    __asm__ volatile("\t pushfq\n" );
    __asm__ volatile("\t popq %rax\n" );
    __asm__ volatile("\t orq $0x200,%rax\n" );
    __asm__ volatile("\t pushq %rax\n" );
    __asm__ volatile("\t pushq $0x2b\n" );
    __asm__ volatile("\t push %0\n" : "=m"(tss->gotoaddr));
    __asm__ volatile("\t iretq\n" );
}

