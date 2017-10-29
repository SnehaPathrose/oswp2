//
// Created by sneha pathrose on 10/28/17.
//
#include <sys/defs.h>
#include <sys/allocator.h>
void *unallocated;
void *end;
void initbump(void *physfree, void *physend) {
    unallocated = physfree;
    end = physend;
}

void *bump(uint64_t size)
{
    void *ret;
    if(unallocated+size>end)
        return 0;
    else
    {
        ret = unallocated;
        unallocated = unallocated+size;
    }
    return ret;
}

void *get_unallocated() {
    return unallocated;
}

