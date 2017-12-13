//
// Created by Toby Babu on 10/22/17.
//
#include <sys/defs.h>
#ifndef COURSEPROJ_VIRTUALMEM_H
#define COURSEPROJ_VIRTUALMEM_H

struct page_table_struct_test {
    uint64_t page_value;
};

struct pml4t {
    struct page_table_struct_test PML4Entry[0x200];
}__attribute__((__packed__))__attribute__((aligned(4096))) *pml4t_t;

struct pdpt {
    struct page_table_struct_test PDPEntry[0x200];
}__attribute__((__packed__))__attribute__((aligned(4096)));

struct pdt {
    struct page_table_struct_test PDEntry[0x200];
}__attribute__((__packed__))__attribute__((aligned(4096)));

struct pt {
    struct page_table_struct_test PageEntry[0x200];
}__attribute__((__packed__))__attribute__((aligned(4096)));


void flush_tlb();
void init_paging(void *physfree);
void map_address(uint64_t address, uint64_t map_phy_address);
struct pml4t* map_user_pml4();
void map_user_address(uint64_t address, uint64_t map_phy_address, int size_to_map, struct pml4t *map_table, uint64_t flags);
struct pml4t* duplicate_page_table(struct pml4t *source_table);
struct pml4t* copy_pml4(struct pml4t *user_table);
void map_address_initial(uint64_t address, uint64_t map_phy_address);
void copy_page(struct pml4t *user_table, uint64_t page);
void map_pagetables();
void unmap_page(uint64_t virtual_address);
#define	KERNBASE	0xffffffff80000000
#define	USERBASE	0xFFFFCCCC00000000

#endif //COURSEPROJ_VIRTUALMEM_H


