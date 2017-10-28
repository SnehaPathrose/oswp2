//
// Created by sneha pathrose on 10/28/17.
//

#ifndef OSWP2_ALLOCATOR_H
#define OSWP2_ALLOCATOR_H
void initbump(void *physfree, void *physend);
void *bump(uint64_t size);

#endif //OSWP2_ALLOCATOR_H
