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

#define INITIAL_STACK_SIZE 4096
uint8_t initial_stack[INITIAL_STACK_SIZE]__attribute__((aligned(16)));
uint32_t* loader_stack;
extern char kernmem, physbase;

void start(uint32_t *modulep, void *physbase, void *physfree)
{
    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    uint64_t physend = 0;
    //register char *temp2, *temp1;
    //uint64_t i = 0;
    //struct page_linked_struct *temp_free_list = &free_list;
    while(modulep[0] != 0x9001) modulep += modulep[1]+2;
    for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {
            kprintf("Available Physical Memory [%p-%p]\n", smap->base, smap->base + smap->length);
            if ((uint64_t)physfree>smap->base && (uint64_t)physfree<(smap->base + smap->length))
                physend = smap->base + smap->length;
            /*if (smap->base >= physfree || ((smap->base + smap->length) > physfree)) {

                for (i = smap->base; i <= (smap->base + smap->length) && i > physfree; i++) {
                    temp_free_list->page = i;
                    temp_free_list = temp_free_list->next;
                }
            }*/
        }
    }
    kprintf("physfree %p\n", (uint64_t)physfree);
    kprintf("Kernmem %p\n", (uint64_t)kernmem);
    kprintf("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
    initbump(physfree, (void *)physend);
    pml4t_table = (struct page_table_struct*)bump(512 * sizeof(struct page_table_struct));
    //pml4t_table->present = 1;
    //first_pml4t = (struct pml4t*)physfree;
    init_paging(physbase, physfree);
    __asm__("\t mov %0, %%rax\n" : "=r"(pml4t_table));
    __asm__("\t mov %rax,%cr3\n" );
    /*__asm__("\t mov %cr0,%rax\n" );
    __asm__("\t or $0x80000000, %eax\n" );
    __asm__("\t mov %rax, %cr0\n" );*/
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
    //init_idt();
    //init_pic();
    //__asm__("sti;");

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
