//
// Created by Toby Babu on 10/27/17.
//
#include <sys/defs.h>
#include <sys/virtualmem.h>
#include <sys/allocator.h>
#include <sys/kprintf.h>
void id_paging(struct page_table_entry *first_pte, uint64_t from, int size){
    from = from & 0xfffff000; // discard bits we don't want
    for(;size>0;from+=4096,size-=4096,first_pte++){
        first_pte->page_value = from|1;     // mark page present.
        first_pte->permission = 1;
    }
}

void map_address(uint64_t address, uint64_t map_phy_address, uint64_t physfree) {
    uint64_t pml4t_index = (address >> 39) & 0x00000000000001ff;
    //struct page_table_struct *pdpte, *pdte, *pte;

    struct page_table_entry* page_table = bump(512 * sizeof(struct page_table_entry));
    struct page_table_struct* page_directory = bump(512 * sizeof(struct page_table_struct));
    struct page_table_struct* pdpt = bump(512 * sizeof(struct page_table_struct));
    //struct page_table_struct* new_page_directory;
    for (int i = 0; i < 512; i++) {
        (page_table + i)->page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        (page_directory + i)->page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        (pdpt + i)->page_value = 0x0;
    }
    page_directory->page_value = (uint64_t)page_table;
    page_directory->present = 1;
    page_directory->permission = 1;
    pdpt->page_value = (uint64_t)page_directory;
    pdpt->present = 1;
    pdpt->permission = 1;
    pml4t_table[pml4t_index].page_value = (uint64_t)pdpt;
    pml4t_table[pml4t_index].present = 1;
    pml4t_table[pml4t_index].permission = 1;
    int *i = bump(sizeof(int));
    uint64_t size = physfree /*+ (17 * 512 * sizeof(struct page_table_entry))*/ - (map_phy_address);

    map_phy_address = map_phy_address & 0xfffff000; // discard bits we don't want
    //struct page_table_entry* page_entry = (struct page_table_entry*)(pte->page_value);
    if (page_table == NULL) {
        return;
    }

    if (size > 0) {
        kprintf("Size is %d", size);
    }


    *i = 0;
    for(; size > 0; map_phy_address+=4096, size-=4096, page_table++, (*i)++){
        if (*i == 512) {
            *i = 0;
            page_directory++;
            kprintf("%x", page_directory);
            //size += 512 * sizeof(struct page_table_entry);
            page_table = bump(512 * sizeof(struct page_table_entry));
            for (int j = 0; j < 512; j++) {
                (page_table + j)->page_value = 0x0;
            }
            page_directory->page_value = (uint64_t)page_table;
            page_directory->page_value = page_directory->page_value & 0xfffff000;
            page_directory->permission = 1;
            page_directory->present = 1;
        }
        page_table->page_value = map_phy_address|1;     // mark page present.
        page_table->permission = 1;
    }
}


void init_paging(void *physbase, void *physfree) {
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
    // identity map the first megabyte
    //id_paging(first_page_table, 0x0, 0x100000);
    map_address(0xffffffff80000000, 0x0, (uint64_t)physfree);
    if (pml4t_table->page_value == 0) {
        return;
    }
}