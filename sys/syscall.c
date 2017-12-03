//
// Created by sneha pathrose on 11/17/17.
//
#include <sys/syscall.h>
#include <sys/io.h>
#include <sys/contextswitch.h>
#include <sys/tarfs.h>
#include <sys/klibc.h>
#include <sys/process.h>
#include <sys/gdt.h>
#include <sys/allocator.h>

const void  *syscalls[MAXSYSCALLS] = {&sys_write, &sys_getpid, &sys_malloc, &sys_getcwd, &sys_opendir,
                                      &sys_readdir, &sys_closedir, &sys_execvpe, &sys_fork, &sys_exit,
                                      &sys_read};
uint64_t sys_getpid()
{
    //kprintf(msg);
    return currentthread->pid;
}
int sys_write(int fd,char *msg,int size)
{
    kprintf(msg);
    return 1;
}
uint64_t sys_malloc(uint64_t size)
{
    uint64_t ret,heapsize;
    struct vm_area_struct *tempvma, *heap=NULL;
    tempvma=currentthread->mm->list_of_vmas;
    while (tempvma!= NULL)
    {
        if(tempvma->vma_type==HEAP)
        {
            if((uint64_t)(currentthread->heap_ptr) < (uint64_t)(tempvma->vma_end)) {
                heap = tempvma;
                break;
            }
        }
        tempvma = tempvma->next;
    }

    if(heap == NULL)
        return 0;
    if(size>(heap->vma_end-heap->vma_start))
    {
        //round size to multiple of pagesize
        heapsize=roundup(size);
        createheap(heapsize,currentthread,0x07);
    }
    if((currentthread->heap_ptr+size)>heap->vma_end)
    {
        createheap(PAGE_SIZE,currentthread,0x07);
    }

    ret = currentthread->heap_ptr;
    currentthread->heap_ptr=currentthread->heap_ptr+size;
    return ret;
}

char *sys_getcwd(char *buf, size_t size)
{
    if(size<kstrlength(currentthread->cwd))
        return NULL;
    buf=currentthread->cwd;
    return buf;
}

DIR *sys_opendir(char *dirname)
{
    DIR *dir=do_opendir(dirname);
    return dir;

}
struct dirent *sys_readdir(DIR *dirp)
{
    struct dirent *rd=do_readdir(dirp);
    return rd;
}

int sys_closedir(DIR *dirp)
{
    int r=do_closedir(dirp);
    return r;
}
int sys_execvpe(const char *file, char *const argv[])
{
    int r=do_execvpe(file, argv);
    return r;
}

int sys_fork() {
    //uint64_t addr=(uint64_t)*((uint64_t *)currentthread->rsp);
    struct PCB *new_process = bump(sizeof(struct PCB));
    struct pml4t *forked_table = duplicate_page_table((struct pml4t*)((uint64_t)currentthread->page_table + KERNBASE));
    flush_tlb();
    new_process->page_table = (struct pml4t*)((uint64_t)forked_table - KERNBASE);
    new_process = copy_process(currentthread, new_process);
    __asm__ volatile("\t mov %%rsp,%0\n" : "=m"(currentthread->rsp));
    currentthread->rsp=currentthread->rsp+24;
    new_process->rsp = (uint64_t) &new_process->kstack[399 - (((uint64_t)&currentthread->kstack[399] - currentthread->rsp) / 8)];

    *(uint64_t *)(new_process->rsp+144)=new_process->ursp;
    new_process->kstack[399 - (((uint64_t)&currentthread->kstack[399] - currentthread->rsp) / 8)-1]=0x0;
    new_process->kstack[399 - (((uint64_t)&currentthread->kstack[399] - currentthread->rsp) / 8)-2]=0x0;
    new_process->kstack[399 - (((uint64_t)&currentthread->kstack[399] - currentthread->rsp) / 8)-3]=0x0;
    new_process->kstack[399 - (((uint64_t)&currentthread->kstack[399] - currentthread->rsp) / 8)-4]=0x0;
    new_process->kstack[399 - (((uint64_t)&currentthread->kstack[399] - currentthread->rsp) / 8)-5]=0x0;
    new_process->kstack[399 - (((uint64_t)&currentthread->kstack[399] - currentthread->rsp) / 8)-6]=0x0;
    new_process->kstack[399 - (((uint64_t)&currentthread->kstack[399] - currentthread->rsp) / 8)-7]=0x0;
    new_process->rsp = (uint64_t) &new_process->kstack[399 - (((uint64_t)&currentthread->kstack[399] - currentthread->rsp) / 8)-7];

    // __asm volatile("movq %0, 168(%%rsp)" : "=r" (new_process->ursp));
    struct PCB *temp = currentthread->next;
    currentthread->next=new_process;
    new_process->next=temp;
    //schedule();
    //switch_to_new_process(currentthread);
    //switch_to(currentthread, new_process);

    return currentthread->pid;
}

void sys_exit(int status)
{
    on_completion_pointer();
}

int sys_read(int fd,char *msg, int size) {
    __asm__ volatile("\t sti\n" );
    //char *ab = bump(fd);
    check_for_ip(fd, msg);
    //kprintf("Outside ip %s", msg);
    return 0;
}

void syscall_handler() {
    uint64_t syscallno;
    uint64_t ret;
    void (*sysaddress)();
    __asm volatile("movq 56(%%rsp), %0" : "=r" (syscallno));
    if (syscallno < MAXSYSCALLS) {
        sysaddress = (void *)syscalls[syscallno];
        __asm__ volatile ( "\tpushq %rdi\n");
        __asm__ volatile ( "\tpushq %rcx\n");
        __asm__ volatile ( "\tpushq %rdx\n");
        __asm__ volatile ( "\tpushq %rsi\n");
        __asm__ volatile ( "\tpushq %rbx\n");
        __asm__ volatile ( "callq *%0;":"=a"(ret) :"r" (sysaddress));
        __asm volatile("movq %0, 96 (%%rsp)" : "=r" (ret));
        __asm__ volatile ( "popq %rbx ");
        __asm__ volatile ( "popq %rsi ");
        __asm__ volatile ( "popq %rdx ");
        __asm__ volatile ( "popq %rcx ");
        __asm__ volatile ( "popq %rdi ");
    }
}