//
// Created by Toby Babu on 11/22/17.
//

#ifndef COURSEPROJ_PROCESS_H
#define COURSEPROJ_PROCESS_H
#include <sys/defs.h>
struct PCB *create_process(uint64_t faddr);
void user_process();
struct vm_area_struct *createheap(uint64_t size, struct PCB *process,uint64_t flags);
int do_execvpe(const char *file, char *const argv[]);
struct PCB* copy_process(struct PCB* current_process, struct PCB* new_process);
int processcount;
#endif //COURSEPROJ_PROCESS_H

