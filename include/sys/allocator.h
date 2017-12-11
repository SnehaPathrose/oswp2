//
// Created by sneha pathrose on 10/28/17.
//

#ifndef OSWP2_ALLOCATOR_H
#define OSWP2_ALLOCATOR_H

#include "defs.h"
struct free_list {
    uint64_t current;
    struct free_list *next;
} *head_free, *head_end, *head_unalloc;
void initbump(void *physbase, void *physfree, void *physend);
void *bump(uint64_t size);
uint64_t get_unallocated();
void init_after_paging();
void *bump_user(uint64_t size);
void *bump_physical(uint64_t size);
void kfree(void *freed_pointer);
void *bump_initial(uint64_t size);
uint64_t get_num_free();
#endif //OSWP2_ALLOCATOR_H
