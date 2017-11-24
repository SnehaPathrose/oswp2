#include <sys/defs.h>
#include <sys/gdt.h>
#include <sys/io.h>
#include <sys/tarfs.h>
#include <sys/ahci.h>
#include <sys/idt.h>
#include <sys/pic.h>
#include <sys/pci.h>
#include <sys/virtualmem.h>
#include <sys/allocator.h>
#include <sys/contextswitch.h>
#include <sys/fs.h>

#define INITIAL_STACK_SIZE 4096
uint8_t initial_stack[INITIAL_STACK_SIZE]__attribute__((aligned(16)));
uint32_t* loader_stack;
extern char kernmem, physbase;
extern void switch_to(struct PCB *, struct PCB *);

void pdef() {
    __asm__ volatile("\t cli\n" );
    while(1);
}

/*void switch_to(struct PCB *me, struct PCB *next) {
    __asm__ volatile("\t mov %%rsp,%0\n" : "=m"(next->kstack[398]));
    __asm__ volatile("\t push %%rdi\n" );
    __asm__ volatile("\t mov %%rsp,%0\n" : "=m"(me->rsp) );
    __asm__ volatile("\t mov %0, %%rsp\n" : "m"(next->rsp) );
    __asm__ volatile("\t pop %rdi\n" );
}*/

int write(int file_descriptor, char *buf, int size, int offset) {
    struct filesys_node *out_node = file_descriptors[file_descriptor]->link_to_inode;
    int retval = write_vfs(out_node, buf, size, offset);
    return retval;
}

void start(uint32_t *modulep, void *physbase, void *physfree)
{
    //void pdef() __attribute__((optimize("-O3")));
    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    uint64_t physend = 0;
    while(modulep[0] != 0x9001) modulep += modulep[1]+2;
    for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {
            kprintf("smaplength is %x, smapbase is %x\n", smap->length,
                   smap->base);
            kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);

            if ((uint64_t)physfree>smap->base && (uint64_t)physfree<(smap->base + smap->length))
                physend = smap->base + smap->length;
        }
    }

    kprintf("physfree %p\n", (uint64_t)physfree);
    kprintf("Kernmem %p\n", (uint64_t)kernmem);
    kprintf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
    initbump(physbase, physfree, (void *)physend);
    pml4t_t = (struct pml4t*)bump(sizeof(struct pml4t));
    for (int i = 0; i < 512; i++) {
        pml4t_t->PML4Entry[i].page_value = 0x0;
    }
    init_paging(physfree);
    kprintf("Current unallocated %x", get_unallocated());
    uint64_t pml4t_t_phy = (uint64_t)pml4t_t - KERNBASE;
    __asm volatile("mov %0, %%cr3":: "r"(pml4t_t_phy));
    __asm__("\t mov %cr0,%rax\n" );
    __asm__("\t or $0x80000000, %eax\n" );
    __asm__("\t mov %rax, %cr0\n" );
    char *temp2;
    init_after_paging();
    for(temp2 = (char*)(KERNBASE + 0xb8001); temp2 < (char*)((KERNBASE + 0xb8000)+160*25); temp2 += 2) *temp2 = 7;

    mainthread = bump(sizeof(struct PCB));
    threadlist=mainthread;
    mainthread->next=NULL;
    mainthread->state=0;
    initialise_file_system();
    context_switch();












    /*struct PCB *curr_task = (struct PCB *)bump(sizeof(struct PCB));
    uint64_t arg12 = 0;
    __asm__ volatile("\t mov %%rsp,%0\n" : "=m"(arg12));
    curr_task->rsp = 5;
    struct PCB *next_task = (struct PCB *)bump(sizeof(struct PCB));
    void (*fun1)() = &pdef;
    next_task->kstack[399] = (uint64_t)fun1;
    next_task->gotoaddr = (uint64_t)fun1;
    next_task->rsp= (uint64_t) &(next_task->kstack[399]);
    struct PCB *mainthread = (struct PCB *)bump(sizeof(struct PCB));
    mainthread->threads = mainthread;
    mainthread->parent = NULL;
    mainthread->next = NULL;
    mainthread->state = 0;
    uint64_t arg1 = 0;
    __asm__("\t mov %%rsp,%0\n" : "=m"(arg1));
    mainthread->rsp = arg1;
    next_task->page_table = map_user_pml4();
    next_task->page_table->PML4Entry[510].page_value = (((uint64_t)next_task->page_table & 0xfffffffffffff000)  - KERNBASE)|7;
    //kprintf("\nPage Table Values %d", sizeof(fun1));
    //map_user_address(USERBASE + (uint64_t)physbase, (uint64_t)fun1 - KERNBASE, 4096, next_task->page_table);
    next_task->page_table = (struct pml4t*)((uint64_t)next_task->page_table - KERNBASE);



    next_task->gotoaddr = (uint64_t)fun1;*/
    //create_file_descriptors();
    //write(1, "testing abc", 0, 0);
    //switch_to_ring_3(next_task);

    //initialise_file_system();
    //checkbus();
    /*for(
            temp1 = "Last pressed glyph", temp2 = (char*)(0xb8ec2);
            *temp1;
            temp1 += 1, temp2 += 2
            ) *temp2 = *temp1;
    temp2 = (char*)(0xb8eec);
    *temp2 = ':';
    
    for(
            temp1 = "Time since boot", temp2 = (char*)(0xb8f62);
            *temp1;
            temp1 += 1, temp2 += 2
            ) *temp2 = *temp1;
    temp2 = (char*)(0xb8f8c);
    *temp2 = ':';

    checkbus();*/
    while(1) {
        schedule();
    };
}

void boot(void)
{
    // note: function changes rsp, local stack variables can't be practically used
    register char *temp1, *temp2;

    for(temp2 = (char*)0xb8001; temp2 < (char*)0xb8000+160*25; temp2 += 2) *temp2 = 7 /* white */;
    __asm__ volatile(
    "cli;"
            "movq %%rsp, %0;"
            "movq %1, %%rsp;"
    :"=g"(loader_stack)
    :"r"(&initial_stack[INITIAL_STACK_SIZE])
    );
    init_gdt();
    init_idt();
    init_pic();
    __asm__("sti;");

    start(
            (uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
            (uint64_t*)&physbase,
            (uint64_t*)(uint64_t)loader_stack[4]
    );
    for(
            temp1 = "!!!!! start() returned !!!!!", temp2 = (char*)0xb8160;
            *temp1;
            temp1 += 1, temp2 += 2
            ) *temp2 = *temp1;
    while(1) __asm__ volatile ("hlt");
}
