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
#include <sys/idt.h>

void initialise_syscalls()
{
    syscalls[0]= &sys_write;
    syscalls[1]=&sys_getpid;
    syscalls[2]=&sys_malloc;
    syscalls[3]=&sys_getcwd;
    syscalls[4]=&sys_opendir;
    syscalls[5]=&sys_readdir;
    syscalls[6]=&sys_closedir;
    syscalls[7]=&sys_execvpe;
    syscalls[8]=&sys_fork;
    syscalls[9]=&sys_exit;
    syscalls[10]=&sys_read;
    syscalls[11]=&sys_waitpid;
    syscalls[12]= &sys_access;
    syscalls[13]=&sys_open;
    syscalls[14]=&sys_close;
    syscalls[15]=&sys_sleep;
    syscalls[16]=&sys_kill;
    syscalls[17]=&sys_chdir;
}

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

int sys_chdir(const char *path) {
    struct filesys_tnode* item = find_file((char *) path);
    if (item == NULL) {
        kprintf("\nUnknown directory\n");
        return -1;
    }
    else {
        if (item->link_to_inode->flags == FS_DIRECTORY) {
            memset(currentthread->cwd, '\0', kstrlength(currentthread->cwd));
            kstrcopy(currentthread->cwd, (char *) path);
            return 0;
        }
        else {
            kprintf("\nNot a directory. Cannot do cd.\n");
            return -1;
        }
    }
}

unsigned int sys_sleep(unsigned int seconds)
{
    long time=gettcount();
    int scount=0;
    __asm__ volatile("\t sti\n" );
    while(1) {
        long t=gettcount();
        if(t-time==18) {
            scount++;
            time=t;
        }
        if(scount>=seconds)
            break;
    }
    return 0;
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
    //kprintf("\nValue of Current heap ptr[%d]: %x", currentthread->pid, currentthread->heap_ptr);
    return ret;
}

char *sys_getcwd(char *buf, size_t size)
{
    if(size<kstrlength(currentthread->cwd))
        return NULL;
    kstrcopy(buf,currentthread->cwd);
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
    int r = do_execvpe(file, argv);
    return r;
}

uint32_t sys_waitpid(uint32_t pid, int *status)
{
    uint32_t parent_id = currentthread->pid;
    int setwait=0;
    currentthread->state = 1;
    struct PCB *temp_list = threadlist, *prev = NULL, *next = NULL;
    while (temp_list != NULL) {
        if (temp_list->ppid == parent_id && parent_id!=0 && temp_list->pid==pid) {
            temp_list->is_wait = 1;
            setwait=1;
        }
        temp_list = temp_list->next;
    }
    if(setwait==1) {
        temp_list = threadlist;
        while (temp_list != NULL) {
            if (temp_list->pid == parent_id) {
                if (prev != NULL) {
                    next = temp_list->next;
                    prev->next = next;
                } else {
                    temp_list = temp_list->next;
                }
                break;
            }
            prev = temp_list;
            temp_list = temp_list->next;
        }

        if (blockedlist == NULL) {
            blockedlist = currentthread;
            currentthread->next = NULL;
        } else {
            currentthread->next = blockedlist;
            blockedlist = currentthread;
        }
    }
    schedule();
    return pid;
}

    int sys_fork() {
        //uint64_t addr=(uint64_t)*((uint64_t *)currentthread->rsp);
        struct PCB *new_process = bump(sizeof(struct PCB));
        new_process = copy_process(currentthread, new_process);
        add_to_proc_list(new_process);
        struct pml4t *forked_table = duplicate_page_table((struct pml4t*)((uint64_t)currentthread->page_table + KERNBASE));
        flush_tlb();
        new_process->page_table = (struct pml4t*)((uint64_t)forked_table - KERNBASE);
        __asm__ volatile("\t mov %%rsp,%0\n" : "=m"(currentthread->rsp));
        currentthread->rsp=currentthread->rsp+24;
        new_process->rsp = (uint64_t) &new_process->kstack[399 - (((uint64_t)&currentthread->kstack[399] - currentthread->rsp) / 8)];

        //*(uint64_t *)(new_process->rsp+144)=new_process->ursp;
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
        if(currentthread->child_process == NULL) {
            currentthread->child_process = new_process;
        } else {
            //currentthread->child_process.
        }
        //schedule();
        //switch_to_new_process(currentthread);
        //switch_to(currentthread, new_process);


        //__asm__ volatile ( "addq $24 ,%rsp ");
        return new_process->pid;
    }

    void sys_exit(int status)
    {
        on_completion_pointer();
    }

    int sys_access(const char *pathname, int mode) {
        if (mode == F_OK) {
            return do_findfile((char *) pathname);
        }
        return -1;
    }
    int sys_open(const char *pathname, int flags)
    {
        int r=do_fopen((char *)pathname);
        return r;
    }
    int sys_close(int fd)
    {
        int r=do_close(fd);
        return r;
    }
    int64_t sys_read(int fd,char *msg, int size) {
        if(fd!=0)
        {
            int64_t r=read_vfs(file_descriptors[fd],msg,size)  ;
            return r;
        }
        else {

            __asm__ volatile("\t sti\n" );
            //char *ab = bump(fd);
            check_for_ip(size, msg);
            //kprintf("Outside ip %s", msg);
            return 0;
        }
    }
    int sys_kill(uint32_t pid, int sig) {
        if (sig == 9) {
            if (pid == currentthread->pid) {
                on_completion_pointer();
            }
            else {
                struct PCB *temp_list = threadlist, *prev = NULL, *next = NULL;
                /*while (temp_list != NULL) {
                    if (temp_list->pid == pid) {
                        temp_list->state = 2;
                    }
                    temp_list = temp_list->next;
                }*/
                while (temp_list != NULL) {
                    if (temp_list->pid == pid) {
                        if (prev != NULL) {
                            next = temp_list->next;
                            prev->next = next;
                        }
                        else {
                            temp_list = temp_list->next;
                        }
                        break;
                    }
                    prev = temp_list;
                    temp_list = temp_list->next;
                }
                char process_name[10];
                int i = 0, intvalue = pid;
                while(intvalue!=0)
                {
                    process_name[i++] = intvalue % 10 + '0';
                    intvalue = intvalue / 10;
                }
                process_name[i] = '\0';
                //char *p_name = kstrcat("/proc/", process_name);
                //int i = 0;
                struct filesys_tnode killproc;
                struct filesys_node *killproci = NULL;
                for (i = 0; i < proc_directory->link_to_inode->num_sub_files; i++) {
                    if (kstrcmp(proc_directory->link_to_inode->sub_files_list[i].name, process_name) == 0) {
                        //kprintf("Found process");
                        killproc = proc_directory->link_to_inode->sub_files_list[i];
                        killproci = killproc.link_to_inode;
                        break;
                    }
                }
                for (; i < proc_directory->link_to_inode->num_sub_files - 1; i++) {
                    //if (kstrcmp(proc_directory->link_to_inode->sub_files_list[i].name, process_name) == 0) {
                    proc_directory->link_to_inode->sub_files_list[i] = proc_directory->link_to_inode->sub_files_list[i + 1];
                    //}
                }
                memset(&proc_directory->link_to_inode->sub_files_list[i], 0, sizeof(struct filesys_tnode));
                proc_directory->link_to_inode->num_sub_files--;
                memset(&killproc, 0, sizeof(struct filesys_tnode));
                memset(killproci, 0, sizeof(struct filesys_node));
                kfree(&killproc);
                kfree(killproci);
                /*struct filesys_tnode *item = find_file(p_name);
                if (item != NULL) {
                    pro
                }*/
            }
        }
        return 0;
    }

    void syscall_handler(void *sysaddress) {
        //uint64_t syscallno;
        uint64_t ret;
        //void *sysaddress;
        //syscallno=currentthread->rax;
        //sysaddress = (void *)syscalls[syscallno];
        // if (syscallno < MAXSYSCALLS) {

        __asm__ volatile("\tpush %rax\n");
        __asm__ volatile("\tpush %rbx\n");
        __asm__ volatile("\tpush %rcx\n");
        __asm__ volatile("\tpush %rdx\n");
        __asm__ volatile("\tpush %rdi\n");
        __asm__ volatile("\tpush %rsi\n");
        __asm__ volatile("\tpush %rbp\n");
        __asm__ volatile( "movq %0,%%rdi":: "m"(currentthread->rdi));
        __asm__ volatile( "movq %0,%%rsi"::"m"(currentthread->rsi));
        __asm__ volatile( "movq %0,%%rdx":: "r"(currentthread->rdx));
        __asm__ volatile( "callq *%0;":"=a"(ret) :"r" (sysaddress):"rdx","rdi","rsi");
        __asm__ volatile("movq %0, 112(%%rsp)" : "=r" (ret));
        __asm__ volatile("\tpop %rbp\n");
        __asm__ volatile("\tpop %rsi\n");
        __asm__ volatile("\tpop %rdi\n");
        __asm__ volatile("\tpop %rdx\n");
        __asm__ volatile("\tpop %rcx\n");
        __asm__ volatile("\tpop %rbx\n");
        __asm__ volatile("\tpop %rax\n");

        //}
    }