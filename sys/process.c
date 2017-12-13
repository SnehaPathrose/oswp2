//
// Created by sneha pathrose on 11/18/17.
//
#include <sys/process.h>
#include <sys/allocator.h>
#include <sys/virtualmem.h>
#include <sys/contextswitch.h>
#include <sys/tarfs.h>
#include <sys/io.h>
#include <sys/klibc.h>
#include <sys/fs.h>
#include <sys/syscall.h>
#include <sys/gdt.h>

void add_to_proc_list(struct PCB *process) {
    char process_name[10];
    int i = 0, intvalue = process->pid;
    while (intvalue != 0) {
        process_name[i++] = intvalue % 10 + '0';
        intvalue = intvalue / 10;
    }
    process_name[i] = '\0';
    struct filesys_tnode *process_node = create_t_node(process_name);
    process_node->link_to_inode->link_to_process = process;
    //kprintf("Num of subfiles: %d", proc_directory->link_to_inode->num_sub_files);
    if (proc_directory->link_to_inode->num_sub_files == 0) {
        proc_directory->link_to_inode->sub_files_list = process_node;
    } else {
        proc_directory->link_to_inode->sub_files_list[proc_directory->link_to_inode->num_sub_files] = *process_node;
    }
    proc_directory->link_to_inode->num_sub_files++;
}

void user_process() {
    processcount = 0;
    //char *test;
    struct PCB *temp, *process = NULL;
    temp = threadlist;
    while (temp != NULL) {
        if (temp->state == 0) {
            process = temp;
            break;
        }
        temp = temp->next;
    }
    if (process != NULL) {
        kstrcopy(currentthread->cwd, "/rootfs/bin/");
        process->page_table = map_user_pml4();
        process->page_table = (struct pml4t *) ((uint64_t) process->page_table - KERNBASE);
        uint64_t pml4 = (uint64_t) process->page_table & ~0xFFF;
        __asm__ volatile("mov %0, %%cr3"::"r"(pml4));
        loadelf("bin/init", process);
        kstrcopy(currentthread->name, "/rootfs/bin/init");
        processcount++;
        process->pid = processcount;
        process->ppid = 0;
        add_to_proc_list(process);
        flush_tlb();
        switch_to_ring_3(process);
    }
}

struct PCB *copy_process(struct PCB *current_process, struct PCB *new_process) {
    int i;
    for (i = 0; i < KSTACKLEN; i++)
        new_process->kstack[i] = current_process->kstack[i];
    kstrcopy(new_process->cwd, current_process->cwd);
    new_process->state = 1;

    new_process->mm = bump(sizeof(struct mm_struct));
    //map_address((uint64_t) new_process->mm, (uint64_t) ((uint64_t) new_process->mm - KERNBASE));
    processcount++;
    new_process->pid = processcount;
    new_process->ppid = current_process->pid;
    new_process->mm->num_of_vmas = current_process->mm->num_of_vmas;
    new_process->mm->list_of_vmas = bump(sizeof(struct vm_area_struct));
    //map_address((uint64_t) new_process->mm->list_of_vmas, (uint64_t) ((uint64_t) new_process->mm->list_of_vmas - KERNBASE));
    struct vm_area_struct *vma, *tempvma = NULL;
    //struct file *tempfile;
    struct vm_area_struct *temp_area_struct = current_process->mm->list_of_vmas;
    while (temp_area_struct != NULL) {
        if (temp_area_struct->vma_type == HEAP) {
            //uint64_t heapsize = temp_area_struct->vma_end - temp_area_struct->vma_start;
            vma = bump(sizeof(struct vm_area_struct));
            vma->vma_type = HEAP;
            vma->vma_file = NULL;
            vma->vma_start = temp_area_struct->vma_start;
            vma->vma_end = temp_area_struct->vma_end;
            //kprintf("\n Value of child heap start: %x", vma->vma_start);
            //kprintf("\n Value of child heap end: %x", vma->vma_end);
            tempvma = new_process->mm->list_of_vmas;
            while (tempvma->next != NULL)
                tempvma = tempvma->next;
            //tempvma = vma;
            vma->prev = tempvma;
            tempvma->next = vma;
            vma->next = NULL;
            new_process->mm->num_of_vmas++;
            new_process->heap_ptr = current_process->heap_ptr;
            new_process->mm->freelist = current_process->mm->freelist;


            // kmemcpy((uint64_t *)temp_area_struct->vma_start,(uint64_t *)vma->vma_start,heapsize);
        } else if (temp_area_struct->vma_type == STACK) {
            vma = bump(sizeof(struct vm_area_struct));
            vma->vma_file = NULL;
            vma->vma_type = STACK;
            vma->vma_start = (uint64_t) temp_area_struct->vma_start;
            vma->vma_end = temp_area_struct->vma_end;
            //kprintf("\n Value of child stack start: %x", vma->vma_start);
            //kprintf("\n Value of child stack end: %x", vma->vma_end);
            //kprintf("\n Value of stack start: %x", vma->vma_start);
            //kprintf("\n Value of stack end: %x", vma->vma_end);
            tempvma = new_process->mm->list_of_vmas;
            while (tempvma->next != NULL)
                tempvma = tempvma->next;
            //tempvma = vma;
            vma->prev = tempvma;
            tempvma->next = vma;
            vma->next = NULL;
            new_process->ursp = currentthread->ursp;
        } else {
            vma = bump(sizeof(struct vm_area_struct));
            //map_address((uint64_t) vma, (uint64_t) ((uint64_t) vma - KERNBASE));
            vma->vma_start = temp_area_struct->vma_start;
            vma->vma_end = temp_area_struct->vma_end;
            vma->vma_flags = temp_area_struct->vma_flags;
            if (temp_area_struct->vma_file != NULL) {
                vma->vma_file = (struct file *) bump(sizeof(struct file));
                //map_address((uint64_t) vma->vma_file, (uint64_t) ((uint64_t) vma->vma_file - KERNBASE));
                vma->vma_file->bss_size = temp_area_struct->vma_file->bss_size;
                vma->vma_file->file_start = temp_area_struct->vma_file->bss_size;
                vma->vma_file->vm_pgoff = temp_area_struct->vma_file->vm_pgoff;
                vma->vma_file->vm_sz = temp_area_struct->vma_file->vm_sz;
            }
            vma->next = NULL;
            if (tempvma == NULL) {
                new_process->mm->list_of_vmas = vma;
                vma->prev = NULL;
            } else {
                vma->prev = tempvma;
            }
            new_process->mm->num_of_vmas++;
            tempvma = vma;

            //vma=vma->next;

        }

        temp_area_struct = temp_area_struct->next;

    }
    return new_process;
}

struct vm_area_struct *createheap(uint64_t size, struct PCB *process, uint64_t flags) {
    struct vm_area_struct *vma, *tempvma;
    uint64_t *heap = bump_user(size);
    // map_address((uint64_t)heap-USERBASE+KERNBASE,(uint64_t)heap-USERBASE);
    //kprintf("\nValue of heap: %x", (uint64_t)heap);
    vma = bump(sizeof(struct vm_area_struct));
    //map_address((uint64_t) vma, (uint64_t) ((uint64_t) vma - KERNBASE));
    vma->vma_type = HEAP;
    vma->vma_file = NULL;
    vma->vma_start = (uint64_t) heap;
    vma->vma_end = (uint64_t) heap + size - 1;
    if (currentthread->mm->freelist == NULL) {
        currentthread->mm->freelist = (malloc_header *) vma->vma_start;
        currentthread->mm->freelist->s.size = (4096 + sizeof(malloc_header) - 1) / sizeof(malloc_header) - 1;
        currentthread->mm->freelist->s.next = NULL;
    }

    //kprintf("\nValue of heap start: %x", vma->vma_start);
    //kprintf("\nValue of heap end: %x", vma->vma_end);
    tempvma = process->mm->list_of_vmas;
    while (tempvma->next != NULL)
        tempvma = tempvma->next;
    //tempvma = vma;
    vma->prev = tempvma;
    tempvma->next = vma;
    vma->next = NULL;
    process->mm->num_of_vmas++;
    process->heap_ptr = (uint64_t) heap;
    return vma;
}

void *do_malloc(uint64_t size) {
    uint64_t num_of_structs;
    malloc_header *prev, *current;
    num_of_structs = (size + sizeof(malloc_header) - 1) / sizeof(malloc_header) + 1;


    prev = currentthread->mm->freelist;


    for (current = prev;; prev = current, current = current->s.next) {
        if (current->s.size >= num_of_structs) {
            if (current->s.size == num_of_structs) {
                prev->s.next = current->s.next;
            } else {
                current->s.size -= num_of_structs;
                current = current + current->s.size;
                current->s.size = num_of_structs;
            }
            //currentthread->heap_ptr=currentthread->heap_ptr+(num_of_structs * sizeof(malloc_header));
            currentthread->mm->freelist = prev;
            return (void *) current + sizeof(malloc_header);
        }
        if (current == currentthread->mm->freelist) {
            //create new heap
        }

    }


}

void do_free(void *ptr) {
    malloc_header *block, *current;
    block = (malloc_header *) ptr - 1;

    if (currentthread->mm->freelist == NULL) {
        currentthread->mm->freelist = block;
        return;
    }
    current = currentthread->mm->freelist;

    for (; current != NULL; current = current->s.next) {
        if (block > current && current->s.next == NULL) {
            if ((current + current->s.size) == block)
                current->s.size += block->s.size;
            else {
                current->s.next = block;

                block->s.next = NULL;
            }
            break;
        } else if (block < current && current->s.next == NULL) {
            if (block + block->s.size == current) {
                block->s.size += current->s.size;
                block->s.next = NULL;
            } else
                block->s.next = current;
            break;
        } else if (block > current && block < current->s.next) {
            if (block + block->s.size == current->s.next) {
                block->s.size += current->s.next->s.size;
                block->s.next = current->s.next->s.next;
            } else
                block->s.next = current->s.next;
            if (current + current->s.size == block) {
                current->s.size += block->s.size;
            } else
                current->s.next = block;

            break;
        }

    }


}

void clear_vmas() {
    struct vm_area_struct *tempvma, *current = NULL/*,*freevma*/;
    tempvma = currentthread->mm->list_of_vmas;
    while (tempvma != NULL) {
        if (tempvma->vma_type == OTHER) {
            //free the tempvma file
            kfree(tempvma->vma_file);
            //tempvma->vma_file = NULL;
        }
        //freevma=tempvma;
        current = tempvma;
        tempvma = tempvma->next;
        kfree(current);
        //free the freevma
        //freevma=NULL;
    }
    kfree(currentthread->mm->list_of_vmas);
    //currentthread->mm->list_of_vmas=NULL;
    //free mm
    //currentthread->mm=NULL;
    kfree(currentthread->mm);
}

void clear_uptentries() {
    struct pml4t *pagetable = (struct pml4t *) ((uint64_t) (currentthread->page_table) + KERNBASE);
    for (int i = 0; i < 512; i++) {
        if (pagetable->PML4Entry[i].page_value & 0x0000000000000004) {
            if (i == 510 || i == 511) {
                continue;
            }

            struct pdpt *pdpt = (struct pdpt *) (KERNBASE + (pagetable->PML4Entry[i].page_value & 0xfffffffffffff000));
            pagetable->PML4Entry[i].page_value = 0x0;
            for (int i = 0; i < 512; i++) {
                if (pdpt->PDPEntry[i].page_value & 0x0000000000000004) {
                    struct pdt *pdt = (struct pdt *) (KERNBASE + (pdpt->PDPEntry[i].page_value & 0xfffffffffffff000));
                    pdpt->PDPEntry[i].page_value = 0x0;
                    for (int i = 0; i < 512; i++) {
                        if (pdt->PDEntry[i].page_value & 0x0000000000000004) {
                            struct pt *pt = (struct pt *) (KERNBASE +
                                                           (pdt->PDEntry[i].page_value & 0xfffffffffffff000));
                            pdt->PDEntry[i].page_value = 0x0;
                            for (int i = 0; i < 512; i++) {
                                if (pt->PageEntry[i].page_value & 0x0000000000000004) {
                                    pt->PageEntry[i].page_value = 0x0;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

void clear_kernelstack() {
    memset(currentthread->kstack, 0, 400);
}

int do_execvpe(const char *file, char *const argv[], char *const envp[]) {
    char filename[20], args[5][40], envs[5][40];
    int i, argc = 0,envc=0;
    char *stack = NULL, *temp = NULL, *argtemp=NULL;
    uint64_t envtemp;
    kstrcopy(filename, (char *) file);
    // memset(args,'\0', sizeof(args));
    for (i = 0; argv[i] != NULL; i++) {
        kstrcopy(args[i], argv[i]);
        argc++;
    }
    kstrcopy(args[argc], "\0");
    for (i = 0; envp[i] != NULL; i++) {
        kstrcopy(envs[i], envp[i]);
        envc++;
    }
    kstrcopy(envs[envc], "\0");
    clear_vmas();
    clear_uptentries();
    loadelf(filename, currentthread);
    stack = (char *) currentthread->ursp;

    for (i = envc - 1; i >= 0; i--) {
        stack = stack - kstrlength(envs[i]) - 1;
        kstrcopy(stack, envs[i]);
    }

    //stack=stack-1;
    // *stack=0;
    argtemp = stack;
    for (i = argc - 1; i >= 0; i--) {
        stack = stack - kstrlength(args[i]) - 1;
        kstrcopy(stack, args[i]);
    }


    temp = (char *) currentthread->ursp;
    currentthread->ursp = (uint64_t) stack;
    currentthread->ursp = currentthread->ursp - 8;
    *(uint64_t *) (currentthread->ursp) = 0;
    for (i = envc; i >= 1; i--) {
        temp = (char *) (temp - kstrlength(envs[i - 1]) - 1);
        *((uint64_t *) currentthread->ursp - (envc - i + 1)) = (uint64_t) temp;
    }
    currentthread->ursp = (uint64_t) ((uint64_t *) currentthread->ursp - envc);
    envtemp = currentthread->ursp;
    currentthread->ursp = currentthread->ursp - 8;
    *(uint64_t *) (currentthread->ursp) = 0;
    for (i = argc; i >= 1; i--) {
        argtemp = (char *) (argtemp - kstrlength(args[i - 1]) - 1);
        *((uint64_t *) currentthread->ursp - (argc - i + 1)) = (uint64_t) argtemp;
    }
    currentthread->ursp = (uint64_t) ((uint64_t *) currentthread->ursp - argc);
    *((uint64_t *) currentthread->ursp - 1) = envtemp;
    *((uint64_t *) currentthread->ursp - 2) = (uint64_t) currentthread->ursp;
    *((uint64_t *) currentthread->ursp - 3) = argc;
    currentthread->ursp = (uint64_t) ((uint64_t *) currentthread->ursp - 3);
    clear_kernelstack();
    set_tss_rsp(&currentthread->kstack[KSTACKLEN - 1]);
    switch_to_ring_3(currentthread);

    return 0;
}

