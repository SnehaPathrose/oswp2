#include <sys/defs.h>
#include <sys/gdt.h>
#include <sys/kprintf.h>
#include <sys/tarfs.h>
#include <sys/ahci.h>
#include <sys/idt.h>
#include <sys/pic.h>
#include <sys/pci.h>
#include <sys/virtualmem.h>
#include <sys/allocator.h>
#include <sys/contextswitch.h>

#define INITIAL_STACK_SIZE 4096
uint8_t initial_stack[INITIAL_STACK_SIZE]__attribute__((aligned(16)));
uint32_t* loader_stack;
extern char kernmem, physbase;
extern void switch_to(struct PCB *, struct PCB *);
void pdef() {
    kprintf("test thread");
}
/*void switch_to(struct PCB *me, struct PCB *next) {
    __asm__ volatile("\t mov %%rsp,%0\n" : "=m"(next->kstack[398]));
    __asm__ volatile("\t push %%rdi\n" );
    __asm__ volatile("\t mov %%rsp,%0\n" : "=m"(me->rsp) );
    __asm__ volatile("\t mov %0, %%rsp\n" : "m"(next->rsp) );
    __asm__ volatile("\t pop %rdi\n" );
}*/
void start(uint32_t *modulep, void *physbase, void *physfree)
{
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

    for(temp2 = (char*)(KERNBASE + 0xb8001); temp2 < (char*)((KERNBASE + 0xb8000)+160*25); temp2 += 2) *temp2 = 7;
    kprintf("Hello");
    //int i,j;
    /*struct task_struct *current_task = bump(sizeof(struct task_struct));
    struct mm_struct *current_mm = bump(sizeof(struct mm_struct));
    struct vm_area_struct *current_vma = bump(sizeof(struct vm_area_struct));
    current_task->active_mm = current_mm;
    current_task->mm = current_mm;
    current_mm->list_of_vmas = current_vma;
    current_mm->num_of_vmas = 1;
    current_mm->page_table = pml4t_t_phy;
    current_vma->vma_start = (uint64_t)bump(4096);
    current_vma->vma_end = current_vma->vma_start + 4096;
    current_vma->next = NULL;
    current_vma->prev = NULL;

    struct task_struct *second_thread = bump(sizeof(struct task_struct));
    second_thread->active_mm = current_mm;
    second_thread->mm = current_mm;*/
    //kprintf("Current unallocated %x", get_unallocated());
    struct PCB *curr_task = bump(sizeof(struct PCB));
   // kprintf("\ninitial stack %s",(char *)initial_stack);
    //for(i=4095,j=399;initial_stack[i]!=0x0;j--,i--)
        //curr_task->kstack[j]=initial_stack[i];
    //curr_task->kstack[] = (char *) &initial_stack;
    uint64_t arg1 = 0;
    __asm__("\t mov %%rsp,%0\n" : "=m"(arg1));
    curr_task->rsp = arg1;
    struct PCB *next_task = bump(sizeof(struct PCB));
    void (*fun1)() = &pdef;
    kprintf("%x",*(fun1));
   //next_task->kstack = bump(sizeof(uint64_t)*5);
   next_task->kstack[398] = (uint64_t)fun1;
    next_task->rsp= (uint64_t) &(next_task->kstack[398]);
    kprintf("addreses %x %x %x\n",next_task,&(next_task->kstack[398]),next_task->kstack[399]);
  //  pdef();
   switch_to(curr_task,next_task);
    kprintf("back");
    //fun1();
    //schedule(curr_task);
    /*//next_task->kstack = bump(4096 * sizeof(next_task->kstack));
    */
    //switch_to(curr_task, next_task);
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
    while(1);
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
