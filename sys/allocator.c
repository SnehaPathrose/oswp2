//
// Created by sneha pathrose on 10/28/17.
//
#include <sys/defs.h>
#include <sys/allocator.h>
#include <sys/virtualmem.h>
#include <sys/io.h>
#include <sys/klibc.h>
#include <sys/contextswitch.h>

void *unallocated;
void *end;
uint64_t number_of_free_pages = 0;
uint64_t num_of_pages;

void initbump(void *physbase, void *physfree, void *physend) {
    unallocated = physfree;
    end = physend;
    int k = 0;
    struct free_list *temp_free = (struct free_list *) physfree;
    head_free = temp_free;

    for (uint64_t i = (uint64_t) physfree, j = (uint64_t) physfree;
         i <= (uint64_t) (physend - 4096); i += 4096, j += 16) {
        temp_free = (struct free_list *) j;
        temp_free->current = i;
        if (i != (uint64_t) (physend - 4096)) {
            temp_free->next = (struct free_list *) (j + 16);
        } else {
            temp_free->next = NULL;
        }
        temp_free = temp_free->next;
        k++;
    }

    int l = (k * 16) / 4096;
    if ((l * 4096) < (k * 16)) {
        l++;
    }

    number_of_free_pages = (uint64_t) (k - l);
    while (l != 0) {
        head_free = head_free->next;
        l--;
    }

}

void init_after_paging() {
    head_free = (struct free_list *) ((uint64_t) head_free + KERNBASE);
    struct free_list *temp_free = head_free;
    while (temp_free->next != NULL) {
        temp_free->next = (struct free_list *) ((uint64_t) temp_free->next + KERNBASE);
        temp_free = temp_free->next;
    }
    head_end = temp_free;
    head_unalloc = head_end;
}

void *bump(uint64_t size) {
    uint64_t ret;
    num_of_pages = size / 4096;
    if (((size / 4096) * 4096) < size) {
        num_of_pages++;
    }
    struct free_list *temp_free = head_free;
    if (num_of_pages > number_of_free_pages) {
        return 0;
    } else {
        ret = head_free->current;
        while (num_of_pages != 0) {
            temp_free = head_free;
            head_free = head_free->next;
            head_end->next = temp_free;
            head_end = head_end->next;
            head_end->next = NULL;
            num_of_pages--;
            number_of_free_pages--;
        };
    }
    //kprintf("free %d ",number_of_free_pages);
    map_address((uint64_t) (ret + KERNBASE), (uint64_t) ret);
    /*if(&currentthread->kstack >= (ret + KERNBASE) && &currentthread->kstack<=(ret + KERNBASE + 4096)) {
        kprintf("corrupted");
        memset((void *) (ret + KERNBASE), 0, 4096);
    }*/
    //memset((void *) (ret + KERNBASE), 0, 4096);
    return (void *) (ret + KERNBASE);
}

void *bump_initial(uint64_t size) {
    uint64_t ret;
    num_of_pages = size / 4096;
    if (((size / 4096) * 4096) < size) {
        num_of_pages++;
    }

    struct free_list *temp_free = head_free;
    if (num_of_pages > number_of_free_pages) {
        return 0;
    } else {
        ret = head_free->current;
        while (num_of_pages != 0) {
            temp_free = head_free;
            head_free = head_free->next;
            head_end->next = temp_free;
            head_end = head_end->next;
            head_end->next = NULL;
            num_of_pages--;
            number_of_free_pages--;
        };
    }
    //  kprintf("free %d ",number_of_free_pages);
    return (void *) (ret + KERNBASE);
}

uint64_t get_unallocated() {
    return head_free->current;
}

uint64_t get_num_free() {
    return number_of_free_pages;
}

void *bump_user(uint64_t size) {
    uint64_t ret;
    //head_free = (struct free_list*)((uint64_t)head_free + KERNBASE);
    num_of_pages = size / 4096;
    if (((size / 4096) * 4096) < size) {
        num_of_pages++;
    }

    struct free_list *temp_free = head_free;
    if (num_of_pages > number_of_free_pages) {
        return 0;
    } else {
        ret = head_free->current;
        while (num_of_pages != 0) {
            temp_free = head_free;
            head_free = head_free->next;
            head_end->next = temp_free;
            head_end = head_end->next;
            head_end->next = NULL;
            num_of_pages--;
            number_of_free_pages--;
        };
    }
    // kprintf("free %d ",number_of_free_pages);
    return (void *) (ret + USERBASE);
}

void *bump_physical(uint64_t size) {
    uint64_t ret;
    //head_free = (struct free_list*)((uint64_t)head_free + KERNBASE);
    num_of_pages = size / 4096;
    if (((size / 4096) * 4096) < size) {
        num_of_pages++;
    }

    struct free_list *temp_free = head_free;
    if (num_of_pages > number_of_free_pages) {
        return 0;
    } else {
        ret = head_free->current;
        while (num_of_pages != 0) {
            temp_free = head_free;
            head_free = head_free->next;
            head_end->next = temp_free;
            head_end = head_end->next;
            head_end->next = NULL;
            num_of_pages--;
            number_of_free_pages--;
        };
    }
    //  kprintf("free %d ",number_of_free_pages);
    return (void *) (ret);
}

void kfree(void *freed_pointer) {
    /*struct free_list* temp_free = head_free;
    while (temp_free->next != NULL) {
        //temp_free->next = (struct free_list*)((uint64_t)temp_free->next + KERNBASE);
        temp_free = temp_free->next;
    }*/
    //struct free_list* new_page = (struct free_list*)freed_pointer;
    uint64_t phy_address = 0;
    if (((uint64_t) freed_pointer & KERNBASE) == KERNBASE) {
        phy_address = (uint64_t) freed_pointer - KERNBASE;
    } else if (((uint64_t) freed_pointer & USERBASE) == USERBASE) {
        phy_address = (uint64_t) freed_pointer - USERBASE;
    } else {
        phy_address = (uint64_t) freed_pointer;
    }
    unmap_page((uint64_t) freed_pointer);
    head_unalloc = head_unalloc->next;
    head_unalloc->current = phy_address;
    number_of_free_pages++;
}