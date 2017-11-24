//
// Created by sneha pathrose on 11/18/17.
//
#include <sys/process.h>
#include <sys/allocator.h>
#include <sys/virtualmem.h>
#include <sys/contextswitch.h>
#include <sys/tarfs.h>

void user_process()
{
    struct PCB *temp, *process=NULL;
    temp=threadlist;
    while(temp!=NULL) {
        if (temp->state == 0) {
            process = temp;
            break;
        }
        temp = temp->next;
    }
    if (process != NULL) {
        process->page_table = map_user_pml4();
        process->page_table = (struct pml4t *) ((uint64_t) process->page_table - KERNBASE);
        uint64_t pml4 = (uint64_t) process->page_table & ~0xFFF;
        __asm__ volatile("mov %0, %%cr3"::"r"(pml4));
        loadelf("bin/sbush", process);
        switch_to_ring_3(process);
    }
}