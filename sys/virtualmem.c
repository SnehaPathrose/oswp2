//
// Created by Toby Babu on 10/27/17.
//
#include <sys/defs.h>
#include <sys/virtualmem.h>
#include <sys/allocator.h>
#include <sys/io.h>
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

void flush_tlb()
{
    __asm__ volatile ("\tmovq %cr3,%rax\n");
    __asm__ volatile ("\tmovq %rax,%cr3\n");
}

void map_address(uint64_t address, uint64_t map_phy_address) {
    uint64_t pml4t_index = (address >> 39) & 0x00000000000001ff;
    uint64_t pdpt_index = (address >> 30) & 0x00000000000001ff;
    uint64_t pdt_index = (address >> 21) & 0x00000000000001ff;
    uint64_t pt_index = (address >> 12) & 0x00000000000001ff;
    struct pt* page_table = 0;
    struct pdt* page_directory = 0;
    struct pdpt* pdpt = 0;
    //uint64_t newaddress;

    if ((pml4t_t->PML4Entry[pml4t_index].page_value & 0x0000000000000001) == 0x0){
        pdpt = bump(sizeof(struct pdpt));
        for (int i = 0; i < 512; i++) {
            pdpt->PDPEntry[i].page_value = 0x0;
        }
        pml4t_t->PML4Entry[pml4t_index].page_value = (((uint64_t)pdpt & 0xfffffffffffff000) - KERNBASE) | 3;
    }
    else {
        pdpt = (struct pdpt *)(KERNBASE + (pml4t_t->PML4Entry[pml4t_index].page_value & 0xfffffffffffff000));
    }

    if ((pdpt->PDPEntry[pdpt_index].page_value & 0x0000000000000001) == 0x0){
        page_directory = bump(sizeof(struct pdt));
        for (int i = 0; i < 512; i++) {
            page_directory->PDEntry[i].page_value = 0x0;
        }
        pdpt->PDPEntry[pdpt_index].page_value = (((uint64_t)page_directory  & 0xfffffffffffff000) - KERNBASE) | 3;
    }
    else {
        page_directory = (struct pdt*)(KERNBASE + (pdpt->PDPEntry[pdpt_index].page_value & 0xfffffffffffff000));
    }
    if ((page_directory->PDEntry[pdt_index].page_value & 0x0000000000000001) == 0x0){
        page_table = bump(sizeof(struct pt));
        for (int i = 0; i < 512; i++) {
            page_table->PageEntry[i].page_value = 0x0;
        }
        page_directory->PDEntry[pdt_index].page_value = (((uint64_t)page_table  & 0xfffffffffffff000) - KERNBASE) | 3;
    }
    else {
        page_table = (struct pt*)(KERNBASE + (page_directory->PDEntry[pdt_index].page_value & 0xfffffffffffff000));
    }
    uint64_t abc = get_unallocated();
    int num_of_entries = ((uint64_t)abc - (map_phy_address))/4096;
    if (((uint64_t)abc - (map_phy_address)) % 512 > 0) {
        num_of_entries++;
    }
    //kprintf("To be allocated %d", num_of_entries);
    int num_of_structs = num_of_entries / 512;
    if (num_of_entries % 512 > 0) {
        num_of_structs++;
    }
    //kprintf("Num of structs to be allocated %d", num_of_structs);
    int size = (uint64_t)get_unallocated() - (map_phy_address) + (num_of_structs - 1) * 4096;
    //kprintf("\nsize %d %d", size, size/4096);
    map_phy_address = map_phy_address & 0xfffff000; // discard bits we don't want

    for(; size >= 0; map_phy_address+=4096, size-=4096, pt_index++){
        if (pdpt_index == 512) {
            pdpt_index = 0;
            pml4t_index++;
            pdpt = bump(sizeof(struct pdpt));
            for (int j = 0; j < 512; j++) {
                pdpt->PDPEntry[j].page_value = 0x0;
            }
            pml4t_t->PML4Entry[pml4t_index].page_value = (((uint64_t)pdpt & 0xfffffffffffff000) - KERNBASE) | 3;
            pdt_index = 512;
        }
        if (pdt_index == 512) {
            pdt_index = 0;
            pdpt_index++;
            page_directory = bump(sizeof(struct pdt));
            for (int j = 0; j < 512; j++) {
                page_directory->PDEntry[j].page_value = 0x0;
            }
            pdpt->PDPEntry[pdpt_index].page_value = (((uint64_t)page_directory & 0xfffffffffffff000) - KERNBASE) | 3;
            pt_index = 512;
        }
        if (pt_index == 512) {
            //newaddress = KERNBASE + map_phy_address;
            pt_index = 0;
            pdt_index++;
            //pdt_index=(newaddress >> 21) & 0x00000000000001ff;
            page_table = bump(sizeof(struct pt));
            for (int j = 0; j < 512; j++) {
                page_table->PageEntry[j].page_value = 0x0;
            }
            page_directory->PDEntry[pdt_index].page_value = (((uint64_t)page_table & 0xfffffffffffff000) - KERNBASE) | 3;
        }
        if ((page_table->PageEntry[pt_index].page_value & 0x0000000000000001) == 0x0){
            page_table->PageEntry[pt_index].page_value = map_phy_address|7;     // mark page present.
        }
        else {
            //kprintf("Already Mapped");
        }

    }

}

void map_user_address(uint64_t address, uint64_t map_phy_address, int size_to_map, struct pml4t *map_table, uint64_t flags) {
    uint64_t pml4t_index = (address >> 39) & 0x00000000000001ff;
    uint64_t pdpt_index = (address >> 30) & 0x00000000000001ff;
    uint64_t pdt_index = (address >> 21) & 0x00000000000001ff;
    uint64_t pt_index = (address >> 12) & 0x00000000000001ff;
    struct pt* page_table = 0;
    struct pdt* page_directory = 0;
    struct pdpt* pdpt = 0;
    //uint64_t newaddress;

    if ((map_table->PML4Entry[pml4t_index].page_value & 0x0000000000000001) == 0x0){
        pdpt = bump(sizeof(struct pdpt));
        for (int i = 0; i < 512; i++) {
            pdpt->PDPEntry[i].page_value = 0x0;
        }
        map_table->PML4Entry[pml4t_index].page_value = (((uint64_t)pdpt & 0xfffffffffffff000) - KERNBASE) | flags;
    }
    else {
        pdpt = (struct pdpt *)(KERNBASE + (map_table->PML4Entry[pml4t_index].page_value & 0xfffffffffffff000));
    }

    if ((pdpt->PDPEntry[pdpt_index].page_value & 0x0000000000000001) == 0x0){
        page_directory = bump(sizeof(struct pdt));
        for (int i = 0; i < 512; i++) {
            page_directory->PDEntry[i].page_value = 0x0;
        }
        pdpt->PDPEntry[pdpt_index].page_value = (((uint64_t)page_directory  & 0xfffffffffffff000) - KERNBASE) | flags;
    }
    else {
        page_directory = (struct pdt*)(KERNBASE + (pdpt->PDPEntry[pdpt_index].page_value & 0xfffffffffffff000));
    }
    if ((page_directory->PDEntry[pdt_index].page_value & 0x0000000000000001) == 0x0){
        page_table = bump(sizeof(struct pt));
        for (int i = 0; i < 512; i++) {
            page_table->PageEntry[i].page_value = 0x0;
        }
        page_directory->PDEntry[pdt_index].page_value = (((uint64_t)page_table  & 0xfffffffffffff000) - KERNBASE) | flags;
    }
    else {
        page_table = (struct pt*)(KERNBASE + (page_directory->PDEntry[pdt_index].page_value & 0xfffffffffffff000));
    }
    //uint64_t abc = get_unallocated();
    int num_of_entries = size_to_map/4096;
    if ((size_to_map) % 512 > 0) {
        num_of_entries++;
    }
    //kprintf("To be allocated %d", num_of_entries);
    int num_of_structs = num_of_entries / 512;
    if (num_of_entries % 512 > 0) {
        num_of_structs++;
    }
    //kprintf("Num of structs to be allocated %d", num_of_structs);
    int size = size_to_map + (num_of_structs - 1) * 4096;
    //kprintf("\nsize %d %d", size, size/4096);
    map_phy_address = map_phy_address & 0xfffff000; // discard bits we don't want

    for(; size >= 0; map_phy_address+=4096, size-=4096, pt_index++){
        if (pdpt_index == 512) {
            pdpt_index = 0;
            pml4t_index++;
            pdpt = bump(sizeof(struct pdpt));
            for (int j = 0; j < 512; j++) {
                pdpt->PDPEntry[j].page_value = 0x0;
            }
            map_table->PML4Entry[pml4t_index].page_value = (((uint64_t)pdpt & 0xfffffffffffff000) - KERNBASE) | flags;
            pdt_index = 512;
        }
        if (pdt_index == 512) {
            pdt_index = 0;
            pdpt_index++;
            page_directory = bump(sizeof(struct pdt));
            for (int j = 0; j < 512; j++) {
                page_directory->PDEntry[j].page_value = 0x0;
            }
            pdpt->PDPEntry[pdpt_index].page_value = (((uint64_t)page_directory & 0xfffffffffffff000) - KERNBASE) | 7;
            pt_index = 512;
        }
        if (pt_index == 512) {
            //newaddress = KERNBASE + map_phy_address;
            pt_index = 0;
            pdt_index++;
            //pdt_index=(newaddress >> 21) & 0x00000000000001ff;
            page_table = bump(sizeof(struct pt));
            for (int j = 0; j < 512; j++) {
                page_table->PageEntry[j].page_value = 0x0;
            }
            page_directory->PDEntry[pdt_index].page_value = (((uint64_t)page_table & 0xfffffffffffff000) - KERNBASE) | 7;
        }
        if ((page_table->PageEntry[pt_index].page_value & 0x0000000000000001) == 0x0){
            page_table->PageEntry[pt_index].page_value = map_phy_address|7;     // mark page present.
        }
        else {
            //kprintf("Already Mapped");
        }

    }
}

void duplicate_pt(struct pt *source_pt) {
    for (int i = 0; i < 512; i++) {
        if(source_pt->PageEntry[i].page_value & 0x0000000000000004) {
            source_pt->PageEntry[i].page_value = ((source_pt->PageEntry[i].page_value) & 0xfffffffffffffffd);
        }
    }
}

void duplicate_pdt(struct pdt *source_pdt) {
    for (int i = 0; i < 512; i++) {
        if(source_pdt->PDEntry[i].page_value & 0x0000000000000004) {
            source_pdt->PDEntry[i].page_value = ((source_pdt->PDEntry[i].page_value) & 0xfffffffffffffffd);
            struct pt *pt = (struct pt *)(KERNBASE + (source_pdt->PDEntry[i].page_value & 0xfffffffffffff000));
            duplicate_pt(pt);
        }
    }
}

void duplicate_pdpt(struct pdpt *source_pdpt) {
    for (int i = 0; i < 512; i++) {
        if(source_pdpt->PDPEntry[i].page_value & 0x0000000000000004) {
            source_pdpt->PDPEntry[i].page_value = ((source_pdpt->PDPEntry[i].page_value) & 0xfffffffffffffffd);
            struct pdt *pdt = (struct pdt *)(KERNBASE + (source_pdpt->PDPEntry[i].page_value & 0xfffffffffffff000));
            duplicate_pdt(pdt);
        }
    }
}

struct pml4t* duplicate_page_table(struct pml4t *source_table) {

    for (int i = 0; i < 512; i++) {
        if(source_table->PML4Entry[i].page_value & 0x0000000000000004) {
            if (i == 510) {
                continue;
            }
            source_table->PML4Entry[i].page_value = ((source_table->PML4Entry[i].page_value) & 0xfffffffffffffffd);
            struct pdpt *pdpt = (struct pdpt *)(KERNBASE + (source_table->PML4Entry[i].page_value & 0xfffffffffffff000));
            duplicate_pdpt(pdpt);
        }
    }
    /*struct pml4t *user_table = (struct pml4t*)bump(sizeof(struct pml4t));
    for (int i = 0; i < 512; i++) {
        user_table->PML4Entry[i].page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        if (source_table->PML4Entry[i].page_value != 0x0)
            user_table->PML4Entry[i].page_value = source_table->PML4Entry[i].page_value;
    }
    user_table->PML4Entry[510].page_value = (((uint64_t)user_table->PML4Entry->page_value & 0xfffffffffffff000)  - KERNBASE)|7;
    //struct pml4t* current_pml4t = (struct pml4t*)(KERNBASE + (uint64_t)user_table);
    struct pdpt* current_pdpt = (struct pdpt *)(KERNBASE + (user_table->PML4Entry[0].page_value & 0xfffffffffffff000));
    if(current_pdpt == NULL) {
        return user_table;
    }*/
    return source_table;
}

struct pml4t* map_user_pml4() {
    struct pml4t *user_table = (struct pml4t*)bump(sizeof(struct pml4t));
    for (int i = 0; i < 512; i++) {
        user_table->PML4Entry[i].page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        if (pml4t_t->PML4Entry[i].page_value != 0x0)
            user_table->PML4Entry[i].page_value = pml4t_t->PML4Entry[i].page_value;
    }

    user_table->PML4Entry[510].page_value = (((uint64_t)user_table->PML4Entry->page_value & 0xfffffffffffff000)  - KERNBASE)|7;

    return user_table;
}

struct pt* copy_pt(struct pt* user_pt) {
    struct pt *new_pt = (struct pt*)bump(sizeof(struct pt));
    for (int i = 0; i < 512; i++) {
        new_pt->PageEntry[i].page_value = 0x0;
    }
    for(int i = 0; i < 512; i++) {
        if((user_pt->PageEntry[i].page_value != 0x0) && (user_pt->PageEntry[i].page_value & 0x0000000000000004)) {
            new_pt->PageEntry[i].page_value = user_pt->PageEntry[i].page_value | 7;
        }
        else if(user_pt->PageEntry[i].page_value != 0x0){
            new_pt->PageEntry[i].page_value = user_pt->PageEntry[i].page_value;
        }
    }
    return new_pt;
}

struct pdt* copy_pdt(struct pdt* user_pdt) {
    struct pdt *new_pdt = (struct pdt*)bump(sizeof(struct pdt));
    for (int i = 0; i < 512; i++) {
        new_pdt->PDEntry[i].page_value = 0x0;
    }
    for(int i = 0; i < 512; i++) {
        if((user_pdt->PDEntry[i].page_value != 0x0) && (user_pdt->PDEntry[i].page_value & 0x0000000000000004)) {
            new_pdt->PDEntry[i].page_value = user_pdt->PDEntry[i].page_value | 7;
            struct pt *pt = (struct pt *)(KERNBASE + (user_pdt->PDEntry[i].page_value & 0xfffffffffffff000));
            new_pdt->PDEntry[i].page_value = (((uint64_t)copy_pt(pt) & 0xfffffffffffff000) - KERNBASE) | 7;
        }
        else if(user_pdt->PDEntry[i].page_value != 0x0){
            new_pdt->PDEntry[i].page_value = user_pdt->PDEntry[i].page_value;
        }
    }
    return new_pdt;
}

struct pdpt* copy_pdpt(struct pdpt* user_pdpt) {
    struct pdpt *new_pdpt = (struct pdpt*)bump(sizeof(struct pdpt));
    for (int i = 0; i < 512; i++) {
        new_pdpt->PDPEntry[i].page_value = 0x0;
    }
    for(int i = 0; i < 512; i++) {
        if((user_pdpt->PDPEntry[i].page_value != 0x0) && (user_pdpt->PDPEntry[i].page_value & 0x0000000000000004)) {
            new_pdpt->PDPEntry[i].page_value = user_pdpt->PDPEntry[i].page_value | 7;
            struct pdt *pdt = (struct pdt *)(KERNBASE + (user_pdpt->PDPEntry[i].page_value & 0xfffffffffffff000));
            new_pdpt->PDPEntry[i].page_value = (((uint64_t)copy_pdt(pdt) & 0xfffffffffffff000) - KERNBASE) | 7;
        }
        else if (user_pdpt->PDPEntry[i].page_value != 0x0){
            new_pdpt->PDPEntry[i].page_value = user_pdpt->PDPEntry[i].page_value;
        }
    }
    return new_pdpt;
}

struct pml4t* copy_pml4(struct pml4t *user_table) {
    struct pml4t *old_table = (struct pml4t*)((uint64_t)user_table + KERNBASE);
    struct pml4t *user_table_new=bump(sizeof(struct pml4t));
    uint64_t flags = 0;
    old_table = (struct pml4t*)((uint64_t)old_table & 0xfffffffffffff000);
    for (int i = 0; i < 512; i++) {
        user_table_new->PML4Entry[i].page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        if (i == 510) {
            continue;
        }
        flags = old_table->PML4Entry[i].page_value & 0xfff;
        if((flags != 0x0) && ((flags & 0x0000000000000002) == 0x0) && (flags & 0x0000000000000005)) {
            user_table_new->PML4Entry[i].page_value = old_table->PML4Entry[i].page_value | 7;
            struct pdpt *pdpt = (struct pdpt *)(KERNBASE + (old_table->PML4Entry[i].page_value & 0xfffffffffffff000));
            user_table_new->PML4Entry[i].page_value = (((uint64_t)copy_pdpt(pdpt) & 0xfffffffffffff000) - KERNBASE) | 7;
        }
        else if((flags != 0x0) && (flags & 0x0000000000000003)) {
            user_table_new->PML4Entry[i].page_value = old_table->PML4Entry[i].page_value | flags;
            //struct pdpt *pdpt = (struct pdpt *)(KERNBASE + (old_table->PML4Entry[i].page_value & 0xfffffffffffff000));
            //user_table_new->PML4Entry[i].page_value = (((uint64_t)copy_pdpt(pdpt) & 0xfffffffffffff000) - KERNBASE) | flags;
        }
        /*if((old_table->PML4Entry[i].page_value != 0x0) && ((old_table->PML4Entry[i].page_value & 0x0000000000000002) == 0x0) && (old_table->PML4Entry[i].page_value & 0x0000000000000005)) {
            user_table_new->PML4Entry[i].page_value = old_table->PML4Entry[i].page_value | 7;
            struct pdpt *pdpt = (struct pdpt *)(KERNBASE + (old_table->PML4Entry[i].page_value & 0xfffffffffffff000));
            user_table_new->PML4Entry[i].page_value = (((uint64_t)copy_pdpt(pdpt) & 0xfffffffffffff000) - KERNBASE) | 7;
        }
        else if((old_table->PML4Entry[i].page_value != 0x0) && (old_table->PML4Entry[i].page_value & 0x0000000000000003)) {
            user_table_new->PML4Entry[i].page_value = old_table->PML4Entry[i].page_value | 7;
            struct pdpt *pdpt = (struct pdpt *)(KERNBASE + (old_table->PML4Entry[i].page_value & 0xfffffffffffff000));
            user_table_new->PML4Entry[i].page_value = (((uint64_t)copy_pdpt(pdpt) & 0xfffffffffffff000) - KERNBASE) | 7;
        }*/
    }
    user_table_new->PML4Entry[510].page_value = (((uint64_t)user_table_new & 0xfffffffffffff000)  - KERNBASE)|3;
    return user_table_new;
}


void init_paging(void *physfree) {
    map_address(KERNBASE, (uint64_t)0);
    pml4t_t->PML4Entry[510].page_value = (((uint64_t)pml4t_t & 0xfffffffffffff000)  - KERNBASE)|3;
}

//void map_page()