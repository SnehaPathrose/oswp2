//
// Created by Toby Babu on 11/2/17.
//
#include <sys/defs.h>
#include <sys/virtualmem.h>
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
    //uint64_t page_table;
};

struct task_struct {
    volatile long state;
    long nice;
    struct mm_struct *mm;
    struct mm_struct *active_mm;
    struct task_struct *next, *prev;
    int pid;
    struct thread *thread_list;
};

struct thread {
    uint64_t    rsp;
    uint64_t    ss;
    uint64_t    kernelrsp;
    uint64_t    kernelSs;
    struct task_struct*  parent;
    uint64_t    priority;
    int         state;
    //ktime_t     sleepTimeDelta;
};

struct PCB {
    uint64_t kstack[400];
    struct mm_struct *mm;
    int pid,ppid, is_wait;
    uint64_t rsp, heap_ptr,ursp;
    enum { RUNNING, SLEEPING, ZOMBIE } state;
    int exit_status;
    struct PCB *child_process;
    struct PCB *next;
    struct PCB *parent;
    uint64_t gotoaddr;
    struct pml4t *page_table;
    char cwd[50];
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
char *check_for_ip(int size_ip, char *ab);
struct PCB *create_thread(uint64_t faddr);
#define KSTACKLEN 400
#define STACK 0x01
#define HEAP 0x02
#define OTHER 0x03
#endif //COURSEPROJ_CONTEXTSWITCH_H

