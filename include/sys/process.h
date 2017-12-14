//
// Created by Toby Babu on 11/22/17.
//

#ifndef COURSEPROJ_PROCESS_H
#define COURSEPROJ_PROCESS_H
#define MAXSLICE 8
#include <sys/defs.h>
typedef union m_header{
    struct{
        union m_header *next;
        uint64_t size;
    }s;
    long x;
}malloc_header;
struct PCB *create_process(uint64_t faddr);
void *do_malloc(uint64_t size);
void do_free(void *ptr);
void user_process();
struct vm_area_struct *createheap(uint64_t size, struct PCB *process,uint64_t flags);
int do_execvpe(const char *file, char *const argv[],char *const envp[]);
struct PCB* copy_process(struct PCB* current_process, struct PCB* new_process);
uint32_t processcount;
void add_to_proc_list(struct PCB* process);

#endif //COURSEPROJ_PROCESS_H




