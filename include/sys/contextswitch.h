//
// Created by Toby Babu on 11/2/17.
//
#include <sys/defs.h>
#include <sys/virtualmem.h>
#ifndef COURSEPROJ_CONTEXTSWITCH_H
#define COURSEPROJ_CONTEXTSWITCH_H

struct vm_area_struct {
    uint64_t vma_start;
    uint64_t vma_end;
    struct vm_area_struct *next, *prev;
    //Need a pointer to mm_struct
    //Need pointer to anon_vma
};

struct mm_struct {
    struct vm_area_struct *list_of_vmas;
    int num_of_vmas;
    uint64_t page_table;
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
    uint64_t pid;
    uint64_t rsp;
    enum { RUNNING, SLEEPING, ZOMBIE } state;
    int exit_status;
};

void schedule(struct PCB *);
#endif //COURSEPROJ_CONTEXTSWITCH_H
