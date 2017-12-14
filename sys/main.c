#include <sys/defs.h>
#include <sys/gdt.h>
#include <sys/io.h>
#include <sys/tarfs.h>
#include <sys/ahci.h>
#include <sys/idt.h>
#include <sys/pic.h>
#include <sys/pci.h>
#include <sys/allocator.h>
#include <sys/fs.h>
#include <sys/syscall.h>

#define INITIAL_STACK_SIZE 4096
uint8_t initial_stack[INITIAL_STACK_SIZE]__attribute__((aligned(16)));
uint32_t *loader_stack;
extern char kernmem, physbase;

void start(uint32_t *modulep, void *physbase, void *physfree) {
    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    uint64_t physend = 0;
    while (modulep[0] != 0x9001) modulep += modulep[1] + 2;
    for (smap = (struct smap_t *) (modulep + 2);
         smap < (struct smap_t *) ((char *) modulep + modulep[1] + 2 * 4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {
            //kprintf("smaplength is %x, smapbase is %x\n", smap->length,
            //smap->base);
            //kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);

            if ((uint64_t) physfree > smap->base && (uint64_t) physfree < (smap->base + smap->length))
                physend = smap->base + smap->length;
        }
    }

    //kprintf("physfree %p\n", (uint64_t)physfree);
    //kprintf("Kernmem %p\n", (uint64_t)kernmem);
    //kprintf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
    initbump(physbase, physfree, (void *) physend);
    pml4t_t = (struct pml4t *) bump_initial(sizeof(struct pml4t));
    for (int i = 0; i < 512; i++) {
        pml4t_t->PML4Entry[i].page_value = 0x0;
    }
    init_paging(physfree);
    //kprintf("Current unallocated %x", get_unallocated());
    uint64_t pml4t_t_phy = (uint64_t) pml4t_t - KERNBASE;
    __asm volatile("mov %0, %%cr3"::"r"(pml4t_t_phy));
    __asm__("\t mov %cr0,%rax\n" );
    __asm__("\t or $0x80000000, %eax\n" );
    __asm__("\t mov %rax, %cr0\n" );
    char *temp2;
    init_after_paging();
    for (temp2 = (char *) (KERNBASE + 0xb8001); temp2 < (char *) ((KERNBASE + 0xb8000) + 160 * 25); temp2 += 2)
        *temp2 = 7;
    map_pagetables();
    mainthread = bump(sizeof(struct PCB));
    threadlist = mainthread;
    mainthread->next = NULL;
    mainthread->state = 0;
    mainthread->totalslice=1;
    mainthread->page_table = (struct pml4t *) ((uint64_t) pml4t_t - KERNBASE);
    currentthread = mainthread;
    initialise_file_system();
    initialise_syscalls();
    context_switch();
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
    while (1);
    /*{
        schedule();
    }*/
}

void boot(void) {
    // note: function changes rsp, local stack variables can't be practically used
    register char *temp1, *temp2;

    for (temp2 = (char *) 0xb8001; temp2 < (char *) 0xb8000 + 160 * 25; temp2 += 2) *temp2 = 7 /* white */;
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
            (uint32_t *) ((char *) (uint64_t) loader_stack[3] + (uint64_t) &kernmem - (uint64_t) &physbase),
            (uint64_t *) &physbase,
            (uint64_t *) (uint64_t) loader_stack[4]
    );
    for (
            temp1 = "!!!!! start() returned !!!!!", temp2 = (char *) 0xb8160;
            *temp1;
            temp1 += 1, temp2 += 2
            )
        *temp2 = *temp1;
    while (1) __asm__ volatile ("hlt");
}


