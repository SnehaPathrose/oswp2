//
// Created by Toby Babu on 10/27/17.
//
#include <sys/defs.h>
#include <sys/virtualmem.h>
#include <sys/allocator.h>
#include <sys/io.h>
#include <sys/klibc.h>
#include <sys/contextswitch.h>

void flush_tlb() {
    __asm__ volatile ("\tmovq %cr3,%rax\n");
    __asm__ volatile ("\tmovq %rax,%cr3\n");
}

void map_pagetables() {
    struct pdpt *pdpt = (struct pdpt *) (KERNBASE + (pml4t_t->PML4Entry[511].page_value & 0xfffffffffffff000));
    struct pdt *pdt = (struct pdt *) (KERNBASE + (pdpt->PDPEntry[510].page_value & 0xfffffffffffff000));
    for (int i = 0; i < 512; i++) {
        struct pt *pt = (struct pt *) (pdt->PDEntry[i].page_value);
        if (pt == 0x0) {
            pt = bump_physical(4096);
            map_address((uint64_t) pt + KERNBASE, (uint64_t) pt);
            for (int j = 0; j < 512; j++) {
                ((struct pt *) ((uint64_t) pt + KERNBASE))->PageEntry[j].page_value = 0x0;
            }
            pdt->PDEntry[i].page_value = ((uint64_t) pt & 0xfffffffffffff000) | 3;

        }
    }

    if (pdpt->PDPEntry[511].page_value == 0x0) {

        //pdt = (struct pdt *) (KERNBASE + (pdpt->PDPEntry[511].page_value & 0xfffffffffffff000));
        pdt = bump_physical(4096);
        map_address((uint64_t) pdt + KERNBASE, (uint64_t) pdt);
        for (int j = 0; j < 512; j++) {
            ((struct pdt *) ((uint64_t) pdt + KERNBASE))->PDEntry[j].page_value = 0x0;
        }
        pdpt->PDPEntry[511].page_value = ((uint64_t) pdt & 0xfffffffffffff000) | 3;
        for (int i = 0; i < 512; i++) {
            struct pt *pt = (struct pt *) (((struct pdt *) ((uint64_t) pdt + KERNBASE))->PDEntry[i].page_value);
            if (pt == 0x0) {
                pt = bump_physical(4096);
                map_address((uint64_t) pt + KERNBASE, (uint64_t) pt);
                for (int j = 0; j < 512; j++) {
                    ((struct pt *) ((uint64_t) pt + KERNBASE))->PageEntry[j].page_value = 0x0;
                }
                ((struct pdt *) ((uint64_t) pdt + KERNBASE))->PDEntry[i].page_value =
                        ((uint64_t) pt & 0xfffffffffffff000) | 3;

            }
        }

    }

}

void map_address_initial(uint64_t address, uint64_t map_phy_address) {
    uint64_t pml4t_index = (address >> 39) & 0x00000000000001ff;
    uint64_t pdpt_index = (address >> 30) & 0x00000000000001ff;
    uint64_t pdt_index = (address >> 21) & 0x00000000000001ff;
    uint64_t pt_index = (address >> 12) & 0x00000000000001ff;
    struct pt *page_table = 0;
    struct pdt *page_directory = 0;
    struct pdpt *pdpt = 0;
    //uint64_t newaddress;

    if ((pml4t_t->PML4Entry[pml4t_index].page_value & 0x0000000000000001) == 0x0) {
        pdpt = bump_initial(sizeof(struct pdpt));
        //map_address((uint64_t) pdpt, (uint64_t) ((uint64_t) pdpt - KERNBASE));
        for (int i = 0; i < 512; i++) {
            pdpt->PDPEntry[i].page_value = 0x0;
        }
        pml4t_t->PML4Entry[pml4t_index].page_value = (((uint64_t) pdpt & 0xfffffffffffff000) - KERNBASE) | 3;
    } else {
        pdpt = (struct pdpt *) (KERNBASE + (pml4t_t->PML4Entry[pml4t_index].page_value & 0xfffffffffffff000));
    }

    if ((pdpt->PDPEntry[pdpt_index].page_value & 0x0000000000000001) == 0x0) {
        page_directory = bump_initial(sizeof(struct pdt));
        for (int i = 0; i < 512; i++) {
            page_directory->PDEntry[i].page_value = 0x0;
        }
        pdpt->PDPEntry[pdpt_index].page_value = (((uint64_t) page_directory & 0xfffffffffffff000) - KERNBASE) | 3;
    } else {
        page_directory = (struct pdt *) (KERNBASE + (pdpt->PDPEntry[pdpt_index].page_value & 0xfffffffffffff000));
    }
    if ((page_directory->PDEntry[pdt_index].page_value & 0x0000000000000001) == 0x0) {
        page_table = bump_initial(sizeof(struct pt));
        for (int i = 0; i < 512; i++) {
            page_table->PageEntry[i].page_value = 0x0;
        }
        page_directory->PDEntry[pdt_index].page_value = (((uint64_t) page_table & 0xfffffffffffff000) - KERNBASE) | 3;
    } else {
        page_table = (struct pt *) (KERNBASE + (page_directory->PDEntry[pdt_index].page_value & 0xfffffffffffff000));
    }
    uint64_t abc = get_unallocated();
    int num_of_entries = (int) (((uint64_t) abc - (map_phy_address)) / 4096);
    if (((uint64_t) abc - (map_phy_address)) % 512 > 0) {
        num_of_entries++;
    }
    //kprintf("To be allocated %d", num_of_entries);
    int num_of_structs = num_of_entries / 512;
    if (num_of_entries % 512 > 0) {
        num_of_structs++;
    }
    //kprintf("Num of structs to be allocated %d", num_of_structs);
    int size = (int) ((uint64_t) get_unallocated() - (map_phy_address) + (num_of_structs - 1) * 4096);
    //kprintf("\nsize %d %d", size, size/4096);
    map_phy_address = map_phy_address & 0xfffff000; // discard bits we don't want

    for (; size >= 0; map_phy_address += 4096, size -= 4096, pt_index++) {
        if (pt_index == 512) {
            //newaddress = KERNBASE + map_phy_address;
            pt_index = 0;
            pdt_index++;
            //pdt_index=(newaddress >> 21) & 0x00000000000001ff;
            page_table = bump_initial(sizeof(struct pt));
            if (pdt_index == 512) {
                pdt_index = 0;
                pdpt_index++;
                page_directory = bump_initial(sizeof(struct pdt));
                if (pdpt_index == 512) {
                    pdpt_index = 0;
                    pml4t_index++;
                    pdpt = bump_initial(sizeof(struct pdpt));
                    if (pml4t_index == 512) {
                        kprintf("No More memory");
                    } else {
                        pml4t_t->PML4Entry[pml4t_index].page_value =
                                (((uint64_t) pdpt & 0xfffffffffffff000) - KERNBASE) | 3;
                        for (int j = 0; j < 512; j++) {
                            pdpt->PDPEntry[j].page_value = 0x0;
                        }
                    }
                }
                pdpt->PDPEntry[pdpt_index].page_value =
                        (((uint64_t) page_directory & 0xfffffffffffff000) - KERNBASE) | 3;
                for (int j = 0; j < 512; j++) {
                    page_directory->PDEntry[j].page_value = 0x0;
                }

                //pt_index = 512;
            }

            page_directory->PDEntry[pdt_index].page_value =
                    (((uint64_t) page_table & 0xfffffffffffff000) - KERNBASE) | 3;
            for (int j = 0; j < 512; j++) {
                page_table->PageEntry[j].page_value = 0x0;
            }
        }
        if ((page_table->PageEntry[pt_index].page_value & 0x0000000000000001) == 0x0) {
            page_table->PageEntry[pt_index].page_value = map_phy_address | 3;     // mark page present.
        } else {
            //kprintf("Already Mapped");
        }

    }

}

void map_address(uint64_t address, uint64_t map_phy_address) {
    uint64_t pml4t_index = (address >> 39) & 0x00000000000001ff;
    uint64_t pdpt_index = (address >> 30) & 0x00000000000001ff;
    uint64_t pdt_index = (address >> 21) & 0x00000000000001ff;
    uint64_t pt_index = (address >> 12) & 0x00000000000001ff;
    struct pt *page_table = 0;
    struct pdt *page_directory = 0;
    struct pdpt *pdpt = 0;
    //uint64_t newaddress;

    if ((pml4t_t->PML4Entry[pml4t_index].page_value & 0x0000000000000001) == 0x0) {
        pdpt = bump_initial(sizeof(struct pdpt));
        //map_address((uint64_t) pdpt, (uint64_t) ((uint64_t) pdpt - KERNBASE));
        pml4t_t->PML4Entry[pml4t_index].page_value = (((uint64_t) pdpt & 0xfffffffffffff000) - KERNBASE) | 3;
        for (int i = 0; i < 512; i++) {
            pdpt->PDPEntry[i].page_value = 0x0;
        }
    } else {
        pdpt = (struct pdpt *) (KERNBASE + (pml4t_t->PML4Entry[pml4t_index].page_value & 0xfffffffffffff000));
    }

    if ((pdpt->PDPEntry[pdpt_index].page_value & 0x0000000000000001) == 0x0) {
        page_directory = bump_initial(sizeof(struct pdt));
        pdpt->PDPEntry[pdpt_index].page_value = (((uint64_t) page_directory & 0xfffffffffffff000) - KERNBASE) | 3;
        for (int i = 0; i < 512; i++) {
            page_directory->PDEntry[i].page_value = 0x0;
        }
    } else {
        page_directory = (struct pdt *) (KERNBASE + (pdpt->PDPEntry[pdpt_index].page_value & 0xfffffffffffff000));
    }
    if ((page_directory->PDEntry[pdt_index].page_value & 0x0000000000000001) == 0x0) {
        page_table = bump_initial(sizeof(struct pt));
        page_directory->PDEntry[pdt_index].page_value = (((uint64_t) page_table & 0xfffffffffffff000) - KERNBASE) | 3;
        for (int i = 0; i < 512; i++) {
            page_table->PageEntry[i].page_value = 0x0;
        }
    } else {
        page_table = (struct pt *) (KERNBASE + (page_directory->PDEntry[pdt_index].page_value & 0xfffffffffffff000));
    }
    uint64_t abc = get_unallocated();
    int num_of_entries = (int) (((uint64_t) abc - (map_phy_address)) / 4096);
    if (((uint64_t) abc - (map_phy_address)) % 512 > 0) {
        num_of_entries++;
    }
    //kprintf("To be allocated %d", num_of_entries);
    int num_of_structs = num_of_entries / 512;
    if (num_of_entries % 512 > 0) {
        num_of_structs++;
    }
    //kprintf("Num of structs to be allocated %d", num_of_structs);
    int size = (int) ((uint64_t) get_unallocated() - (map_phy_address) + (num_of_structs - 1) * 4096);
    //kprintf("\nsize %d %d", size, size/4096);
    map_phy_address = map_phy_address & 0xfffff000; // discard bits we don't want

    for (; size > 0; map_phy_address += 4096, size -= 4096, pt_index++) {
        if (pdpt_index == 512) {
            pdpt_index = 0;
            pml4t_index++;
            pdpt = bump(sizeof(struct pdpt));
            for (int j = 0; j < 512; j++) {
                pdpt->PDPEntry[j].page_value = 0x0;
            }
            pml4t_t->PML4Entry[pml4t_index].page_value = (((uint64_t) pdpt & 0xfffffffffffff000) - KERNBASE) | 3;
            pdt_index = 512;
        }
        if (pdt_index == 512) {
            pdt_index = 0;
            pdpt_index++;
            page_directory = bump(sizeof(struct pdt));
            for (int j = 0; j < 512; j++) {
                page_directory->PDEntry[j].page_value = 0x0;
            }
            pdpt->PDPEntry[pdpt_index].page_value = (((uint64_t) page_directory & 0xfffffffffffff000) - KERNBASE) | 3;
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
            page_directory->PDEntry[pdt_index].page_value =
                    (((uint64_t) page_table & 0xfffffffffffff000) - KERNBASE) | 3;
        }
        if ((page_table->PageEntry[pt_index].page_value & 0x0000000000000001) == 0x0) {
            page_table->PageEntry[pt_index].page_value = map_phy_address | 3;     // mark page present.
        } else {
            //kprintf("Already Mapped");
        }

    }

}

void
map_user_address(uint64_t address, uint64_t map_phy_address, int size_to_map, struct pml4t *map_table, uint64_t flags) {
    uint64_t pml4t_index = (address >> 39) & 0x00000000000001ff;
    uint64_t pdpt_index = (address >> 30) & 0x00000000000001ff;
    uint64_t pdt_index = (address >> 21) & 0x00000000000001ff;
    uint64_t pt_index = (address >> 12) & 0x00000000000001ff;
    struct pt *page_table = 0;
    struct pdt *page_directory = 0;
    struct pdpt *pdpt = 0;
    //uint64_t newaddress;

    if ((map_table->PML4Entry[pml4t_index].page_value & 0x0000000000000001) == 0x0) {
        pdpt = bump(sizeof(struct pdpt));
        map_table->PML4Entry[pml4t_index].page_value = (((uint64_t) pdpt & 0xfffffffffffff000) - KERNBASE) | flags;
        for (int i = 0; i < 512; i++) {
            pdpt->PDPEntry[i].page_value = 0x0;
        }
    } else {
        pdpt = (struct pdpt *) (KERNBASE + (map_table->PML4Entry[pml4t_index].page_value & 0xfffffffffffff000));
    }

    if ((pdpt->PDPEntry[pdpt_index].page_value & 0x0000000000000001) == 0x0) {
        page_directory = bump(sizeof(struct pdt));
        pdpt->PDPEntry[pdpt_index].page_value = (((uint64_t) page_directory & 0xfffffffffffff000) - KERNBASE) | flags;
        for (int i = 0; i < 512; i++) {
            page_directory->PDEntry[i].page_value = 0x0;
        }
    } else {
        page_directory = (struct pdt *) (KERNBASE + (pdpt->PDPEntry[pdpt_index].page_value & 0xfffffffffffff000));
    }
    if ((page_directory->PDEntry[pdt_index].page_value & 0x0000000000000001) == 0x0) {
        page_table = bump(sizeof(struct pt));
        page_directory->PDEntry[pdt_index].page_value =
                (((uint64_t) page_table & 0xfffffffffffff000) - KERNBASE) | flags;
        for (int i = 0; i < 512; i++) {
            page_table->PageEntry[i].page_value = 0x0;
        }
    } else {
        page_table = (struct pt *) (KERNBASE + (page_directory->PDEntry[pdt_index].page_value & 0xfffffffffffff000));
    }
    //uint64_t abc = get_unallocated();
    int num_of_entries = size_to_map / 4096;
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

    for (; size >= 0; map_phy_address += 4096, size -= 4096, pt_index++) {
        if (pdpt_index == 512) {
            pdpt_index = 0;
            pml4t_index++;
            pdpt = bump(sizeof(struct pdpt));
            for (int j = 0; j < 512; j++) {
                pdpt->PDPEntry[j].page_value = 0x0;
            }
            map_table->PML4Entry[pml4t_index].page_value = (((uint64_t) pdpt & 0xfffffffffffff000) - KERNBASE) | flags;
            pdt_index = 512;
        }
        if (pdt_index == 512) {
            pdt_index = 0;
            pdpt_index++;
            page_directory = bump(sizeof(struct pdt));
            for (int j = 0; j < 512; j++) {
                page_directory->PDEntry[j].page_value = 0x0;
            }
            pdpt->PDPEntry[pdpt_index].page_value = (((uint64_t) page_directory & 0xfffffffffffff000) - KERNBASE) | 7;
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
            page_directory->PDEntry[pdt_index].page_value =
                    (((uint64_t) page_table & 0xfffffffffffff000) - KERNBASE) | 7;
        }
        if ((page_table->PageEntry[pt_index].page_value & 0x0000000000000001) == 0x0) {
            page_table->PageEntry[pt_index].page_value = map_phy_address | 7;     // mark page present.
        } else {
            //kprintf("Already Mapped");
        }

    }
}

void duplicate_pt(struct pt *source_pt, struct pt *new_pt) {
    for (int i = 0; i < 512; i++) {
        if (source_pt->PageEntry[i].page_value > 0) {
            source_pt->PageEntry[i].page_value = ((source_pt->PageEntry[i].page_value) & 0xfffffffffffffffd);
            source_pt->PageEntry[i].page_value = ((source_pt->PageEntry[i].page_value) | 0x200);
            new_pt->PageEntry[i].page_value = ((source_pt->PageEntry[i].page_value) & 0xfffffffffffff000);
            new_pt->PageEntry[i].page_value = ((source_pt->PageEntry[i].page_value) | 0x205);
        }
    }
}

void duplicate_pdt(struct pdt *source_pdt, struct pdt *new_pdt) {
    for (int i = 0; i < 512; i++) {
        //if(source_pdt->PDEntry[i].page_value & 0x0000000000000004) {
        if (source_pdt->PDEntry[i].page_value > 0) {
            source_pdt->PDEntry[i].page_value = ((source_pdt->PDEntry[i].page_value) & 0xfffffffffffffffd);
            source_pdt->PDEntry[i].page_value = ((source_pdt->PDEntry[i].page_value) | 0x200);
            struct pt *pt = (struct pt *) (KERNBASE + (source_pdt->PDEntry[i].page_value & 0xfffffffffffff000));
            struct pt *new_pt = (struct pt *) bump(sizeof(struct pt));
            for (int i = 0; i < 512; i++) {
                new_pt->PageEntry[i].page_value = 0x0;
            }
            new_pdt->PDEntry[i].page_value = ((uint64_t) new_pt - KERNBASE) & 0xfffffffffffff000;
            new_pdt->PDEntry[i].page_value = new_pdt->PDEntry[i].page_value | 0x205;
            duplicate_pt(pt, new_pt);
        }
        //}
    }
}

void duplicate_pdpt(struct pdpt *source_pdpt, struct pdpt *new_pdpt) {
    for (int i = 0; i < 512; i++) {
        if (source_pdpt->PDPEntry[i].page_value > 0) {
            source_pdpt->PDPEntry[i].page_value = ((source_pdpt->PDPEntry[i].page_value) & 0xfffffffffffffffd);
            source_pdpt->PDPEntry[i].page_value = ((source_pdpt->PDPEntry[i].page_value) | 0x200);
            struct pdt *pdt = (struct pdt *) (KERNBASE + (source_pdpt->PDPEntry[i].page_value & 0xfffffffffffff000));
            struct pdt *new_pdt = (struct pdt *) bump(sizeof(struct pdt));
            for (int i = 0; i < 512; i++) {
                new_pdt->PDEntry[i].page_value = 0x0;
            }
            new_pdpt->PDPEntry[i].page_value = ((uint64_t) new_pdt - KERNBASE) & 0xfffffffffffff000;
            new_pdpt->PDPEntry[i].page_value = new_pdpt->PDPEntry[i].page_value | 0x205;
            duplicate_pdt(pdt, new_pdt);
        }
    }
}

struct pml4t *duplicate_page_table(struct pml4t *source_table) {

    struct pml4t *user_table = (struct pml4t *) bump(sizeof(struct pml4t));
    for (int i = 0; i < 512; i++) {
        user_table->PML4Entry[i].page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        if (source_table->PML4Entry[i].page_value > 0) {
            if (i == 510) {
                continue;
            }
            if (i == 511) {
                user_table->PML4Entry[i].page_value = source_table->PML4Entry[i].page_value;
            } else {
                source_table->PML4Entry[i].page_value = ((source_table->PML4Entry[i].page_value) & 0xfffffffffffffffd);
                source_table->PML4Entry[i].page_value = ((source_table->PML4Entry[i].page_value) | 0x200);
                struct pdpt *pdpt = (struct pdpt *) (KERNBASE +
                                                     (source_table->PML4Entry[i].page_value & 0xfffffffffffff000));
                struct pdpt *new_pdpt = (struct pdpt *) bump(sizeof(struct pdpt));
                for (int i = 0; i < 512; i++) {
                    new_pdpt->PDPEntry[i].page_value = 0x0;
                }
                user_table->PML4Entry[i].page_value = ((uint64_t) new_pdpt - KERNBASE) & 0xfffffffffffff000;
                user_table->PML4Entry[i].page_value = user_table->PML4Entry[i].page_value | 0x205;
                duplicate_pdpt(pdpt, new_pdpt);
            }
        }
    }
    user_table->PML4Entry[510].page_value = (((uint64_t) user_table & 0xfffffffffffff000) - KERNBASE) | 3;
    return user_table;
}

struct pml4t *map_user_pml4() {
    struct pml4t *user_table = (struct pml4t *) bump(sizeof(struct pml4t));
    for (int i = 0; i < 512; i++) {
        user_table->PML4Entry[i].page_value = 0x0;
    }
    for (int i = 0; i < 512; i++) {
        if (pml4t_t->PML4Entry[i].page_value != 0x0)
            user_table->PML4Entry[i].page_value = pml4t_t->PML4Entry[i].page_value;
    }

    user_table->PML4Entry[510].page_value =
            (((uint64_t) user_table->PML4Entry->page_value & 0xfffffffffffff000) - KERNBASE) | 7;

    return user_table;
}

void copy_page(struct pml4t *user_table, uint64_t page) {
    uint64_t pml4t_index = (page >> 39) & 0x00000000000001ff;
    uint64_t pdpt_index = (page >> 30) & 0x00000000000001ff;
    uint64_t pdt_index = (page >> 21) & 0x00000000000001ff;
    uint64_t pt_index = (page >> 12) & 0x00000000000001ff;
    struct pt *page_table = 0;
    struct pdt *page_directory = 0;
    struct pdpt *pdpt = 0;
    //uint64_t newaddress;

    user_table->PML4Entry[pml4t_index].page_value = user_table->PML4Entry[pml4t_index].page_value | 7;
    if ((user_table->PML4Entry[pml4t_index].page_value & 0x0000000000000001) == 0x0) {
        pdpt = bump(sizeof(struct pdpt));
        //map_address((uint64_t) pdpt, (uint64_t) ((uint64_t) pdpt - KERNBASE));
        for (int i = 0; i < 512; i++) {
            pdpt->PDPEntry[i].page_value = 0x0;
        }
        user_table->PML4Entry[pml4t_index].page_value = (((uint64_t) pdpt & 0xfffffffffffff000) - KERNBASE) | 7;
    } else {
        pdpt = (struct pdpt *) (KERNBASE + (user_table->PML4Entry[pml4t_index].page_value & 0xfffffffffffff000));
        pdpt->PDPEntry[pdpt_index].page_value = pdpt->PDPEntry[pdpt_index].page_value | 7;
    }

    if ((pdpt->PDPEntry[pdpt_index].page_value & 0x0000000000000001) == 0x0) {
        page_directory = bump(sizeof(struct pdt));
        for (int i = 0; i < 512; i++) {
            page_directory->PDEntry[i].page_value = 0x0;
        }
        pdpt->PDPEntry[pdpt_index].page_value = (((uint64_t) page_directory & 0xfffffffffffff000) - KERNBASE) | 7;
    } else {
        page_directory = (struct pdt *) (KERNBASE + (pdpt->PDPEntry[pdpt_index].page_value & 0xfffffffffffff000));
        page_directory->PDEntry[pdt_index].page_value = page_directory->PDEntry[pdt_index].page_value | 7;
    }
    if ((page_directory->PDEntry[pdt_index].page_value & 0x0000000000000001) == 0x0) {
        page_table = bump(sizeof(struct pt));
        for (int i = 0; i < 512; i++) {
            page_table->PageEntry[i].page_value = 0x0;
        }
        page_directory->PDEntry[pdt_index].page_value = (((uint64_t) page_table & 0xfffffffffffff000) - KERNBASE) | 7;
    } else {
        page_table = (struct pt *) (KERNBASE + (page_directory->PDEntry[pdt_index].page_value & 0xfffffffffffff000));
    }
    uint64_t phy_value = (uint64_t) bump_physical(4096);
    if (page_table->PageEntry[pt_index].page_value & 0x0000000000000001) {
        void *copy_value = bump(4096);
        kmemcpychar((void *) (page & 0xfffffffffffff000), copy_value, 4096);
        page_table->PageEntry[pt_index].page_value = phy_value | 7;
        flush_tlb();
        kmemcpychar(copy_value, (void *) (page & 0xfffffffffffff000), 4096);
        kfree(copy_value);
    } else {
        page_table->PageEntry[pt_index].page_value = phy_value | 7;
        flush_tlb();
    }
    //flush_tlb();
}

void unmap_page(uint64_t virtual_address) {
    uint64_t pml4t_index = (virtual_address >> 39) & 0x00000000000001ff;
    uint64_t pdpt_index = (virtual_address >> 30) & 0x00000000000001ff;
    uint64_t pdt_index = (virtual_address >> 21) & 0x00000000000001ff;
    uint64_t pt_index = (virtual_address >> 12) & 0x00000000000001ff;
    struct pml4t *current_table = ((struct pml4t *) ((uint64_t) currentthread->page_table + KERNBASE));
    struct pdpt *pdpt = (struct pdpt *) (KERNBASE +
                                         (current_table->PML4Entry[pml4t_index].page_value & 0xfffffffffffff000));
    struct pdt *pdt = (struct pdt *) (KERNBASE + (pdpt->PDPEntry[pdpt_index].page_value & 0xfffffffffffff000));
    struct pt *pt = (struct pt *) (KERNBASE + (pdt->PDEntry[pdt_index].page_value & 0xfffffffffffff000));
    pt->PageEntry[pt_index].page_value = 0x0;
}

void init_paging(void *physfree) {
    map_address_initial(KERNBASE, (uint64_t) 0);
    pml4t_t->PML4Entry[510].page_value = (((uint64_t) pml4t_t & 0xfffffffffffff000) - KERNBASE) | 3;
}

//void map_page()