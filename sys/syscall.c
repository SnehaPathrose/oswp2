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


void initialise_syscalls() {
    /*syscalls[0] = &sys_write;
    syscalls[1] = &sys_getpid;
    syscalls[2] = &sys_malloc;
    syscalls[3] = &sys_getcwd;
    syscalls[4] = &sys_opendir;
    syscalls[5] = &sys_readdir;
    syscalls[6] = &sys_closedir;
    syscalls[7] = &sys_execvpe;
    syscalls[8] = &sys_fork;
    syscalls[9] = &sys_exit;
    syscalls[10] = &sys_read;
    syscalls[11] = &sys_waitpid;
    syscalls[12] = &sys_access;
    syscalls[13] = &sys_open;
    syscalls[14] = &sys_close;
    syscalls[15] = &sys_sleep;
    syscalls[16] = &sys_kill;
    syscalls[17] = &sys_chdir;
    syscalls[18] = &sys_free;
    syscalls[19] = &sys_setenv;
    syscalls[20] = &sys_getenv;*/
}

pid_t sys_getpid() {
    pid_t pid = currentthread->pid;
    return pid;
}

pid_t sys_getppid() {
    pid_t ppid = currentthread->ppid;
    return ppid;
}

int sys_write(int fd, char *msg, int size) {
    int64_t r = write_vfs(file_descriptors[fd], msg, size, 0);
    return (int) r;
}

int sys_chdir(const char *path) {
    struct filesys_tnode *item = find_file((char *) path);
    if (item == NULL) {
        kprintf("\nUnknown directory\n");
        return -1;
    } else {
        if (item->link_to_inode->flags == FS_DIRECTORY) {
            memset(currentthread->cwd, '\0', kstrlength(currentthread->cwd));
            kstrcopy(currentthread->cwd, (char *) path);
            if (currentthread->ppid != 0) {
                struct PCB *temp;
                temp = threadlist;
                while (temp != NULL) {
                    if (temp->pid == currentthread->ppid) {
                        memset(temp->cwd, '\0', kstrlength(temp->cwd));
                        kstrcopy(temp->cwd, (char *) path);
                    }
                    temp = temp->next;
                }
                temp = blockedlist;
                while (temp != NULL) {
                    if (temp->pid == currentthread->ppid) {
                        memset(temp->cwd, '\0', kstrlength(temp->cwd));
                        kstrcopy(temp->cwd, (char *) path);
                    }
                    temp = temp->next;
                }
            }
            return 0;
        } else {
            kprintf("\nNot a directory. Cannot do cd.\n");
            return -1;
        }
    }
}

unsigned int sys_sleep(unsigned int seconds) {
    long time = gettcount();
    int scount = 0;
    __asm__ volatile("\t sti\n" );
    while (1) {
        long t = gettcount();
        if (t - time == 18) {
            scount++;
            time = t;
        }
        if (scount >= seconds)
            break;
    }
    __asm__ volatile("\t cli\n" );
    return 0;
}

uint64_t sys_malloc(uint64_t size) {
    uint64_t ret = (uint64_t) do_malloc(size);
    return ret;
}

void sys_free(void *ptr) {
    do_free(ptr);
}

char *sys_getcwd(char *buf, size_t size) {
    if (size < kstrlength(currentthread->cwd))
        return NULL;
    kstrcopy(buf, currentthread->cwd);
    return buf;
}

DIR *sys_opendir(char *dirname) {
    DIR *dir = do_opendir(dirname);
    return dir;

}

struct dirent *sys_readdir(DIR *dirp) {
    struct dirent *rd = do_readdir(dirp);
    return rd;
}

int sys_closedir(DIR *dirp) {
    int r = do_closedir(dirp);
    return r;
}

int sys_execvpe(const char *file, char *const argv[], char *const envp[]) {
    /*uint64_t unalloc = get_num_free();
    kprintf("unallocated pages %d", unalloc);*/
    kstrcopy(currentthread->name, (char *) file);
    int r = do_execvpe(file, argv, envp);
    return r;
}

/*uint32_t sys_wait(int *status) {
    uint32_t parent_id = currentthread->pid;
    int setwait = 0;
    currentthread->state = 1;
    struct PCB *temp_list = threadlist, *prev = NULL, *next = NULL;
    while (temp_list != NULL) {
        if (temp_list->ppid == parent_id && parent_id != 0) {
            temp_list->is_wait = 1;
            setwait = 1;
        }
        temp_list = temp_list->next;
    }
    if (setwait == 1) {
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
    currentthread->state = 3;
    schedule();
    return 0;
}*/

uint32_t sys_waitpid(uint32_t pid, int *status) {
    uint32_t parent_id = currentthread->pid;
    int setwait = 0;
    currentthread->state = 1;
    struct PCB *temp_list = threadlist, *prev = NULL, *next = NULL;
    while (temp_list != NULL) {
        if (temp_list->ppid == parent_id && parent_id != 0 && temp_list->pid == pid) {
            temp_list->is_wait = 1;
            setwait = 1;
        }
        temp_list = temp_list->next;
    }
    if (setwait == 1) {
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
    currentthread->state = 3;
    schedule();
    return pid;
}

int sys_fork() {

    struct PCB *new_process = bump(sizeof(struct PCB));
    new_process = copy_process(currentthread, new_process);
    add_to_proc_list(new_process);

    struct pml4t *forked_table = duplicate_page_table(
            (struct pml4t *) ((uint64_t) currentthread->page_table + KERNBASE));

    new_process->page_table = (struct pml4t *) ((uint64_t) forked_table - KERNBASE);

    __asm__ volatile("\t mov %%rsp,%0\n" : "=m"(currentthread->rsp));

    currentthread->rsp = currentthread->rsp + 24;
    uint64_t r12value = 0;
    __asm__ volatile("mov %%r12,%0":"=r"(r12value));
    flush_tlb();
    new_process->rsp = (uint64_t) &new_process->kstack[KSTACKLEN - 1 - (((uint64_t) &currentthread->kstack[KSTACKLEN - 1] - currentthread->rsp) /8)];

    //*(uint64_t *)(new_process->rsp+144)=new_process->ursp
    new_process->kstack[KSTACKLEN - 1 - (((uint64_t) &currentthread->kstack[KSTACKLEN - 1] - currentthread->rsp) / 8) -
                        1] = 0x0;
    new_process->kstack[KSTACKLEN - 1 - (((uint64_t) &currentthread->kstack[KSTACKLEN - 1] - currentthread->rsp) / 8) -
                        2] = 0x0;
    new_process->kstack[KSTACKLEN - 1 - (((uint64_t) &currentthread->kstack[KSTACKLEN - 1] - currentthread->rsp) / 8) -
                        3] = 0x0;
    new_process->kstack[KSTACKLEN - 1 - (((uint64_t) &currentthread->kstack[KSTACKLEN - 1] - currentthread->rsp) / 8) -
                        4] = 0x0;
    new_process->kstack[KSTACKLEN - 1 - (((uint64_t) &currentthread->kstack[KSTACKLEN - 1] - currentthread->rsp) / 8) -
                        5] = 0x0;
    new_process->kstack[KSTACKLEN - 1 - (((uint64_t) &currentthread->kstack[KSTACKLEN - 1] - currentthread->rsp) / 8) -
                        6] = 0x0;
    new_process->kstack[KSTACKLEN - 1 - (((uint64_t) &currentthread->kstack[KSTACKLEN - 1] - currentthread->rsp) / 8) -
                        7] = 0x0;
    new_process->kstack[KSTACKLEN - 1 - (((uint64_t) &currentthread->kstack[KSTACKLEN - 1] - currentthread->rsp) / 8) -
                        8] = r12value;
    new_process->rsp = (uint64_t) &new_process->kstack[KSTACKLEN - 1 -
                                                       (((uint64_t) &currentthread->kstack[KSTACKLEN - 1] -
                                                         currentthread->rsp) / 8) - 8];

    // __asm volatile("movq %0, 168(%%rsp)" : "=r" (new_process->ursp));
    struct PCB *temp = currentthread->next;
    currentthread->next = new_process;
    new_process->next = temp;
    if (currentthread->child_process == NULL) {
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

void sys_exit(int status) {
    on_completion_pointer();
}

int sys_access(const char *pathname, int mode) {
    char concatstr[25];
    char *fullpath = kstrcat("/rootfs/", (char *) pathname, concatstr);
    if (mode == F_OK) {
        int ret_val = do_findfile(fullpath);
        return ret_val;
    }
    return -1;
}

int sys_open(const char *pathname, int flags) {
    int r = do_fopen((char *) pathname);
    return r;
}

int sys_close(int fd) {
    int r = do_close(fd);
    return r;
}

int64_t sys_read(int fd, char *msg, int size) {
    if (fd != 0) {
        int64_t r = read_vfs(file_descriptors[fd], msg, size);
        return r;
    } else {

        __asm__ volatile("\t sti\n" );
        //char *ab = bump(fd);
        check_for_ip(msg, size, 0);
        //int64_t r=file_descriptors[fd]->link_to_inode->read(msg, size, 0);
        //kprintf("Outside ip %s", msg);
        __asm__ volatile("\t cli\n" );
        return 0;
    }
}

int sys_kill(uint32_t pid, int sig) {
    if (sig == 9) {
        if (pid == 2) {
            kprintf("You are attempting to kill the sbush process. \nThe OS will hang! \nAre you sure you want to do that? (y/n) ");
            char value[3];
            sys_read(0, value, 2);
            if ((value[0] == 'Y') | (value[0] == 'y')) {

            } else {
                return -1;
            }
        }
        if (pid == currentthread->pid) {
            on_completion_pointer();
        } else {
            struct PCB *temp_list = threadlist, *prev = NULL, *next = NULL, *current = NULL;
            /*while (temp_list != NULL) {
                if (temp_list->pid == pid) {
                    temp_list->state = 2;
                }
                temp_list = temp_list->next;
            }*/
            while (temp_list != NULL) {
                if (temp_list->pid == pid) {
                    current = temp_list;
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
            if (current == NULL) {
                temp_list = blockedlist;
                prev = NULL;
                next = NULL;
                while (temp_list != NULL) {
                    if (temp_list->pid == pid) {
                        current = temp_list;
                        if (prev != NULL) {
                            next = temp_list->next;
                            prev->next = next;
                        } else {
                            if (temp_list->next != NULL) {
                                temp_list = temp_list->next;
                                blockedlist = temp_list;
                            } else {
                                blockedlist = NULL;
                            }
                        }
                        break;
                    }
                    prev = temp_list;
                    temp_list = temp_list->next;
                }
            }

            if (current == NULL) {
                kprintf("No process with that pid found.");
            } else {
                //memset(current, 0, sizeof(struct PCB));
                char process_name[10];
                int i = 0, intvalue = pid;
                while (intvalue != 0) {
                    process_name[i++] = intvalue % 10 + '0';
                    intvalue = intvalue / 10;
                }
                process_name[i] = '\0';
                //char *p_name = kstrcat("/proc/", process_name);
                //int i = 0;
                struct filesys_tnode *killproc = NULL;
                struct filesys_node *killproci = NULL;
                for (i = 0; i < proc_directory->link_to_inode->num_sub_files; i++) {
                    if (kstrcmp(proc_directory->link_to_inode->sub_files_list[i].name, process_name) == 0) {
                        //kprintf("Found process");
                        killproc = &proc_directory->link_to_inode->sub_files_list[i];
                        killproci = killproc->link_to_inode;
                        break;
                    }
                }
                for (; i < proc_directory->link_to_inode->num_sub_files - 1; i++) {
                    //if (kstrcmp(proc_directory->link_to_inode->sub_files_list[i].name, process_name) == 0) {
                    proc_directory->link_to_inode->sub_files_list[i] = proc_directory->link_to_inode->sub_files_list[i +
                                                                                                                     1];
                    //}
                }
                memset(&proc_directory->link_to_inode->sub_files_list[i], 0, sizeof(struct filesys_tnode));
                proc_directory->link_to_inode->num_sub_files--;
                kfree(killproci);
            }
        }
    }
    return 0;
}

void syscall_handler(void *sysaddress) {
    uint64_t ret;

    __asm__ volatile("\tpush %rax\n");
    __asm__ volatile("\tpush %rbx\n");
    __asm__ volatile("\tpush %rcx\n");
    __asm__ volatile("\tpush %rdx\n");
    __asm__ volatile("\tpush %rdi\n");
    __asm__ volatile("\tpush %rsi\n");
    __asm__ volatile("\tpush %rbp\n");
    __asm__ volatile("\tpush %r12\n");
    __asm__ volatile( "movq %0,%%rdi"::"m"(currentthread->rdi));
    __asm__ volatile( "movq %0,%%rsi"::"m"(currentthread->rsi));
    __asm__ volatile( "movq %0,%%rdx"::"r"(currentthread->rdx));
    __asm__ volatile( "callq *%0;":"=a"(ret) :"r" (sysaddress):"rdx", "rdi", "rsi");
    __asm__ volatile("movq %0, 128(%%rsp)" : "=r" (ret));
    __asm__ volatile("\tpop %r12\n");
    __asm__ volatile("\tpop %rbp\n");
    __asm__ volatile("\tpop %rsi\n");
    __asm__ volatile("\tpop %rdi\n");
    __asm__ volatile("\tpop %rdx\n");
    __asm__ volatile("\tpop %rcx\n");
    __asm__ volatile("\tpop %rbx\n");
    __asm__ volatile("\tpop %rax\n");

    //}
}

