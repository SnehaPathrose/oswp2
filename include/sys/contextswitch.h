//
// Created by Toby Babu on 11/2/17.
//
#include <sys/defs.h>
#include <sys/virtualmem.h>
#include <sys/process.h>

#ifndef COURSEPROJ_CONTEXTSWITCH_H
#define COURSEPROJ_CONTEXTSWITCH_H

struct file{
    uint64_t   file_start;       /* start address of region */
    uint64_t   vm_pgoff;         /* offset in file or NULL */
    uint64_t   vm_sz;            /* region initialised to here */
    uint64_t   bss_size;
};
struct vm_area_struct {
    uint64_t vma_start;
    uint64_t vma_end;
    uint32_t vma_flags;
    uint32_t vma_type;
    struct vm_area_struct *next, *prev;
    struct file *vma_file;
    //Need a pointer to mm_struct
    //Need pointer to anon_vma
};

struct mm_struct {
    struct vm_area_struct *list_of_vmas;
    int num_of_vmas;
    malloc_header *freelist;
    //uint64_t page_table;
};

#define KSTACKLEN 400

struct PCB {
    uint64_t kstack[KSTACKLEN];
    struct mm_struct *mm;
    uint32_t pid,ppid;
    int is_wait;
    uint64_t rsp, heap_ptr,ursp;
    enum { RUNNING, SLEEPING, ZOMBIE, BLOCKED } state;
    //int exit_status;
    struct PCB *child_process;
    struct PCB *next;
    // struct PCB *parent;
    uint64_t gotoaddr;
    struct pml4t *page_table;
    uint64_t rax;
    uint64_t rbx;
    uint64_t rdx;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rcx;
    uint64_t rbp;
    //uint64_t r12;
    char cwd[25];
    char name[25];
};

struct PCB *mainthread;
struct PCB *threadlist;
struct PCB *currentthread;
struct PCB *blockedlist;
void schedule();
void switch_to(struct PCB *me, struct PCB *next);
void context_switch();
void switch_to_ring_3(struct PCB *tss);
void idle_func();
void (*on_completion_pointer)();
void switch_to_new_process(struct PCB *tss);
struct PCB* create_duplicate_process(struct PCB* forked_process);
struct PCB *create_thread(uint64_t faddr);

#define STACK 0x01
#define HEAP 0x02
#define OTHER 0x03
#endif //COURSEPROJ_CONTEXTSWITCH_H





