//
// Created by Toby Babu on 10/27/17.
//
#include <sys/defs.h>
#include <sys/virtualmem.h>
#include <sys/allocator.h>
#include <sys/kprintf.h>
void id_paging(uint64_t from, int size){
    struct page_table_entry* first_page_table = bump(512 * sizeof(struct page_table_entry));
    struct page_table_struct* first_page_directory = bump(512 * sizeof(struct page_table_struct));
    struct page_table_struct* first_pdpt = bump(512 * sizeof(struct page_table_struct));
    for (int i = 0; i < 512; i++) {
        (first_page_table + i)->page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        (first_page_directory + i)->page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        (first_pdpt + i)->page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        (pml4t_table + i)->page_value = 0x0;
    }
    first_page_directory->page_value = (uint64_t)first_page_table;
    first_page_directory->present = 1;
    first_page_directory->permission = 1;
    first_pdpt->page_value = (uint64_t)first_page_directory;
    first_pdpt->present = 1;
    first_pdpt->permission = 1;
    pml4t_table->page_value = (uint64_t)first_pdpt;
    pml4t_table->present = 1;
    pml4t_table->permission = 1;
    from = from & 0xfffff000; // discard bits we don't want
    for(;size>0;from+=4096,first_page_table++){
        first_page_table->page_value = from|1;     // mark page present.
        first_page_table->permission = 1;
        size = size - 4096;
    }
}

void map_address(uint64_t address, uint64_t map_phy_address, uint64_t physfree) {
    uint64_t pml4t_index = (address >> 39) & 0x00000000000001ff;
    uint64_t pdpt_index = (address >> 30) & 0x00000000000001ff;
    uint64_t pdt_index = (address >> 21) & 0x00000000000001ff;
    uint64_t pt_index = (address >> 12) & 0x00000000000001ff;
    struct pt* page_table = bump(sizeof(struct pt));
    struct pdt* page_directory = bump(sizeof(struct pdt));
    struct pdpt* pdpt = bump(sizeof(struct pdpt));
    for (int i = 0; i < 512; i++) {
        pml4t_t->PML4Entry[i].page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        page_table->PageEntry[i].page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        page_directory->PDEntry[i].page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        pdpt->PDPEntry[i].page_value = 0x0;
    }
    page_directory->PDEntry[pdt_index].page_value = (((uint64_t)page_table  & 0xfffffffffffff000) - KERNBASE) | 3;
    pdpt->PDPEntry[pdpt_index].page_value = (((uint64_t)page_directory  & 0xfffffffffffff000) - KERNBASE) | 3;
    pml4t_t->PML4Entry[pml4t_index].page_value = (((uint64_t)pdpt & 0xfffffffffffff000) - KERNBASE) | 3;
    int size = (uint64_t)get_unallocated() - (map_phy_address) + 4096;
    kprintf("\nsize %d %d",size,size/4096);
    map_phy_address = map_phy_address & 0xfffff000; // discard bits we don't want

    for(; size >= 0; map_phy_address+=4096, size-=4096, pt_index++){
        if (pt_index == 512) {
            pt_index = 0;
            pdt_index++;
            page_table = bump(sizeof(struct pt));
            for (int j = 0; j < 512; j++) {
                page_table->PageEntry[j].page_value = 0x0;
            }
            page_directory->PDEntry[pdt_index].page_value = (((uint64_t)page_table & 0xfffffffffffff000) - KERNBASE) | 3;
        }
        page_table->PageEntry[pt_index].page_value = map_phy_address|3;     // mark page present.
    }


}


void init_paging(void *physbase, void *physfree) {

    map_address(KERNBASE, (uint64_t)0, (uint64_t)physfree + (uint64_t)get_unallocated());
    pml4t_t->PML4Entry[510].page_value = (((uint64_t)pml4t_t & 0xfffffffffffff000)  - KERNBASE)|3;
}