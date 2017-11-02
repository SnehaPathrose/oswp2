//
// Created by sneha pathrose on 10/28/17.
//

#ifndef OSWP2_ALLOCATOR_H
#define OSWP2_ALLOCATOR_H
struct free_list {
    uint64_t current;
    struct free_list *next;
} *head_free;
void initbump(void *physbase, void *physfree, void *physend);
void *bump(uint64_t size);
void *get_unallocated();

#endif //OSWP2_ALLOCATOR_H
