//
// Created by Toby Babu on 10/22/17.
//
#include <sys/defs.h>
#include <sys/kprintf.h>
#ifndef COURSEPROJ_VIRTUALMEM_H
#define COURSEPROJ_VIRTUALMEM_H

struct page_table_entry {
    union {
        uint64_t page_value;
        struct {
            uint64_t present:1;
            uint64_t permission:1; //0 for write not allowed
            uint64_t user_access:1;
            uint64_t ignored:2;
            uint64_t accessed:1;
            uint64_t dirty:1;
            uint64_t page_access_type:1;
            uint64_t global:1;
            uint64_t ignored1:3;
            uint64_t pageframe:36;
            uint64_t ignored2:16;
        };
    };
} __attribute__((aligned(4096)));

struct page_table_struct_test {
    uint64_t page_value;
};

struct page_table_struct {
    union {
        uint64_t page_value;
        struct {
            uint64_t present:1;
            uint64_t permission:1; //0 for write not allowed
            uint64_t user_access:1;
            uint64_t ignored:2;
            uint64_t accessed:1;
            uint64_t ignored1:1;
            uint64_t page_size:1;
            uint64_t ignored2:4;
            uint64_t pageframe:36;
            uint64_t ignored3:16;
        };
    };
} __attribute__((aligned(4096))) *pml4t_table;

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

void init_paging(void *physbase, void *physfree);

#define	KERNBASE	0xffffffff80000000

#endif //COURSEPROJ_VIRTUALMEM_H
