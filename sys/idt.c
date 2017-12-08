#include <sys/idt.h>
#include <sys/defs.h>
#include <sys/virtualmem.h>
#include <sys/io.h>
#include <sys/syscall.h>
#include <sys/contextswitch.h>
#include <sys/allocator.h>

#define MAX_IDT 255

struct idtr_t {
    uint16_t size; // size of the idt table
    uint64_t addr; // address of the idt table
}__attribute__((packed));

struct idtr_item {
    uint16_t offset_1; // offset bits 0..15
    uint16_t selector; // a code segment selector in GDT or LDT
    uint8_t ist;       // bits 0..2 holds Interrupt Stack Table offset, rest of bits zero.
    uint8_t type_attr; // type and attributes
    uint16_t offset_2; // offset bits 16..31
    uint32_t offset_3; // offset bits 32..63
    uint32_t zero;     // reserved
}__attribute__((packed));

struct idtr_item idt[MAX_IDT];
/*const static int scantochar[100] = { '\0','\0','1','2','3','4','5','6','7','8','9','0','\0','\0','\0','\0',
                             'q','w','e','r','t','y','u','i','o','p','\0','\0','\0','\0',
                             'a','s','d','f','g','h','j','k','l','\0','\0','\0','\0','\0','z','x',
                             'c','v','b','n','m' };
const static int scantoUchar[100] = { '\0','\0','!','@','#','$','%','^','&','*','(',')','\0','\0','\0','\0',
                                   'Q','W','E','R','T','Y','U','I','O','P','\0','\0','\0','\0',
                                   'A','S','D','F','G','H','J','K','L','\0','\0','\0','\0','\0','Z','X',
                                   'C','V','B','N','M' };
const static int scantoCchar[100] = { '\0','\0','1','2','3','4','5','6','7','8','9','0','\0','\0','\0','\0',
                                   'Q','W','E','R','T','Y','U','I','O','P','\0','\0','\0','\0',
                                   'A','S','D','F','G','H','J','K','L','\0','\0','\0','\0','\0','Z','X',
                                   'C','V','B','N','M' };*/
//static int controlflag = 0x00;
static int scount = 0;   // count in seconds
static int mcount = 0;   // count in minutes
static int hcount = 0;   // count in hours
static long mscount = 0; // count in timer frequency
static long tcount=0;
/*
 * Function:  handletime
 * --------------------
 * Calculates the time in hh:mm:ss format
 */
void handletime() {
    register char *temp2 = (char *)(KERNBASE + 0xb8f90);
    int intvalue, i = 0, rem[2];
    rem[0] = 0 + '0';
    rem[1] = 0 + '0';
    if (scount == 60) {
        mcount++;
        scount = 0;

        if (mcount == 60) {
            hcount++;
            mcount = 0;
        }
        if (hcount == 24)
            hcount = 0;

    }

    if (mscount == 18) {
        scount++;
        mscount = 0;
    }

    intvalue = hcount;
    while(intvalue != 0)
    {
        rem[i++] = intvalue % 10 + '0';
        intvalue = intvalue / 10;
    }

    for(int j = 1;j >= 0;temp2 += 2, j--)
        *temp2 = rem[j];

    *temp2 = ':';
    temp2 += 2;
    intvalue = mcount;
    rem[0] = 0 + '0';
    rem[1] = 0 + '0';
    i = 0;

    while(intvalue != 0)
    {
        rem[i++] = intvalue % 10 + '0';
        intvalue = intvalue / 10;
    }

    for(int j = 1;j >= 0;temp2 += 2, j--)
        *temp2 = rem[j];

    *temp2 = ':';
    temp2 += 2;
    intvalue = scount;
    rem[0] = 0 + '0';
    rem[1] = 0 + '0';
    i = 0;

    while(intvalue != 0)
    {
        rem[i++] = intvalue % 10 + '0';
        intvalue = intvalue / 10;
    }

    for(int j = 1;j >= 0;temp2 += 2, j--)
        *temp2 = rem[j];
}

/*
 * Function:  interrupt0
 * --------------------
 * Interrupt handler for the timer interrupt
 * Computes the boot time in seconds, minutes and hours
 */
void interrupt0() {
    tcount++;
    mscount++;
    // save the registers
    __asm__("\tpush %rax\n");
    __asm__("\tpush %rcx\n");
    __asm__("\tpush %rdx\n");
    __asm__("\tpush %rsi\n");
    __asm__("\tpush %rdi\n");
    __asm__("\tpush %r8\n");
    __asm__("\tpush %r9\n");
    handletime();
    // get the saved registers
    __asm__("\tpop %r9\n");
    __asm__("\tpop %r8\n");
    __asm__("\tpop %rdi\n");
    __asm__("\tpop %rsi\n");
    __asm__("\tpop %rdx\n");
    __asm__("\tpop %rcx\n");
    __asm__("\tpop %rax\n");
    __asm__("\tpop %r11\n");

    __asm__ volatile ( "out %0, %1" : : "a"(0x20), "Nd"(0x20) );
    __asm__( "\tiretq\n");
};

long gettcount()
{
    return tcount;
}

/*
 * Function:  interrupt1
 * --------------------
 * Interrupt handler for the keyboard interrupt
 * Keeps track of the key which was pressed and writes to the screen.
 * Also keeps track of control, shift and capslock characters through control flag
 */
void interrupt1() {
    //register char *temp2 = (char *)(KERNBASE + 0xb8ef0);
    uint8_t keyscancode;
    //char retval[3];
    __asm__ volatile ( "inb %1, %0": "=a"(keyscancode): "Nd"(0x60) );

    __asm__("\tpush %rax\n");
    __asm__("\tpush %rcx\n");
    __asm__("\tpush %rdx\n");
    __asm__("\tpush %rsi\n");
    __asm__("\tpush %rdi\n");
    __asm__("\tpush %r8\n");
    __asm__("\tpush %r9\n");
    kscanf(keyscancode);
    __asm__("\tpop %r9\n");
    __asm__("\tpop %r8\n");
    __asm__("\tpop %rdi\n");
    __asm__("\tpop %rsi\n");
    __asm__("\tpop %rdx\n");
    __asm__("\tpop %rcx\n");
    __asm__("\tpop %rax\n");
    __asm__("\tpop %r11\n");
    /*if ((keyscancode & 0x80) == 0)
    {
        temp2 += 2;
        *temp2 = ' ';
        temp2 = (char *)(KERNBASE + 0xb8ef0);
        if (keyscancode == 58) //caps lock
        {
            if (controlflag & 0x01)
            {
                controlflag=controlflag & 0xfe;
            }
            else
                controlflag=controlflag | 0x01;
        }
        else
        if (keyscancode == 42 || keyscancode == 54) //shift
        {
            controlflag = controlflag | 0x02;
        }
        else
        if (keyscancode == 29) // control
        {
                controlflag = controlflag | 0x04;
        }
        else {

            if (controlflag & 0x04) //ctrl
            {
                *temp2 = '^';
                temp2 += 2;
                *temp2 = scantoCchar[keyscancode];
                controlflag = controlflag & 0xfb;
            }
            else
            if (controlflag & 0x02) //shift
            {
                *temp2 = scantoUchar[keyscancode];
                controlflag = controlflag & 0xfd;
            }
            else
            if (controlflag & 0x01) //caps lock
            {
                *temp2 = scantoUchar[keyscancode];
            }
            else {
                *temp2 = scantochar[keyscancode];
            }
        }
    }*/

    __asm__ volatile ( "out %0, %1" : : "a"(0x20), "Nd"(0x20) );
    __asm__( "\tiretq\n");
};

/*
 * Function:  interrupt2
 * --------------------
 * Interrupt handler for all the exceptions
 * Any exceptions which arise are simply acknowledged
 * This function has to be changed in the course project to
 * handle these exceptions
 */
void interrupt2() {
    __asm__ volatile ( "out %0, %1" : : "a"(0x20), "Nd"(0x20) );
    __asm__( "\tiretq\n");
};

/*
 * Function:  interrupt3
 * --------------------
 * Interrupt handler for all the interrupts other than 0 and 1
 * Any interrupts which arise are simply acknowledged
 */
void interrupt3() {
    __asm__ volatile ( "out %0, %1" : : "a"(0x20), "Nd"(0x20) );
    __asm__( "\tiretq\n");
};

/*
 * Function:  interrupt_syscall
 * --------------------
 * Interrupt handler for all syscalls
 */
void interrupt_syscall() {
    __asm__("\tpush %rax\n");
    __asm__("\tpush %rbx\n");
    __asm__("\tpush %rcx\n");
    __asm__("\tpush %rdx\n");
    __asm__("\tpush %rdi\n");
    __asm__("\tpush %rsi\n");
    __asm__("\tpush %rbp\n");
    __asm volatile("movq 88(%%rsp), %0" : "=g" (currentthread->ursp));
    __asm__ volatile("movq 48(%%rsp), %0": "=r"(currentthread->rax));
    __asm__ volatile("movq 24(%%rsp),%0": "=r"(currentthread->rdx));
    __asm__ volatile("movq 8(%%rsp),%0": "=r"(currentthread->rsi));
    __asm__ volatile("movq 16(%%rsp),%0": "=r"(currentthread->rdi));
    void *sysaddress=(void *)syscalls[currentthread->rax];
    syscall_handler(sysaddress);
    //schedule();
    //__asm__ volatile ( "callq %0;"::"r" (&syscall_handler):"rdx");
    // get the saved registers
    __asm__("\tpop %rbp\n");
    __asm__("\tpop %rsi\n");
    __asm__("\tpop %rdi\n");
    __asm__("\tpop %rdx\n");
    __asm__("\tpop %rcx\n");
    __asm__("\tpop %rbx\n");
    __asm__("\tpop %rax\n");
    __asm__("\taddq $8,%rsp\n");
    __asm__ volatile ("movq %rax,%r9");
    __asm__ volatile ( "out %0, %1" : : "a"(0x20), "Nd"(0x20) );
    __asm__ volatile ("movq %r9,%rax");
    __asm__( "\tiretq\n");

}

void interrupt5() {
    kprintf("General Protection Fault\n");

    uint64_t faultAddr;
    uint64_t cr3;

    __asm volatile("mov %%cr2, %0" : "=r" (faultAddr));
    __asm volatile("mov %%cr3, %0" : "=r" (cr3));

    kprintf("\n gpf cr3 %p\n", cr3);
    kprintf("\n gpf cr2 %p\n", faultAddr);

    while(1);
    __asm__ volatile ( "out %0, %1" : : "a"(0x20), "Nd"(0x20) );
    __asm__( "\tiretq\n");
};

int is_cow_enabled(uint64_t pml4t_index) {
    if ((currentthread != 0x0) &&
        (currentthread->page_table != 0x0) &&
        (((struct pml4t *)((uint64_t)currentthread->page_table + KERNBASE))->PML4Entry[pml4t_index].page_value & 0x0000000000000200) && //COW bit set
        ((((struct pml4t *)((uint64_t)currentthread->page_table + KERNBASE))->PML4Entry[pml4t_index].page_value & 0x0000000000000001) == 0x1)) { //Present bit set
        return 1;
    }
    return 0;
}

/*
 * Function:  interrupt4
 * --------------------
 * Interrupt handler for all the interrupts other than 0 and 1
 * Any interrupts which arise are simply acknowledged
 */
void interrupt4(struct pt_regs *regs) {
    uint64_t *arg1 = 0;
    __asm__ volatile ( "pushq %rdi ");
    __asm__ volatile ( "pushq %rax ");
    __asm__ volatile ( "pushq %rbx ");
    __asm__ volatile ( "pushq %rcx ");
    __asm__ volatile ( "pushq %rdx ");
    __asm__ volatile ( "pushq %rbp ");
    __asm__ volatile ( "pushq %rsi ");
    __asm__ volatile ( "pushq %r8 ");
    __asm__ volatile ( "pushq %r9 ");
    __asm__ volatile ("movq %%cr2, %0;" :"=a"(arg1));
    //if (arg1 & 0x)
    uint64_t pml4t_index = (((uint64_t)arg1 >> 39) & 0x00000000000001ff);
    //struct pml4t *current_page_table =  ((struct pml4t *)((uint64_t)currentthread->page_table + KERNBASE));
    if (is_cow_enabled(pml4t_index)) {

        //currentthread->page_table = (struct pml4t *) ((((uint64_t)copy_pml4(currentthread->page_table) & 0xfffffffffffff000) - KERNBASE));
        //uint64_t pml4 = (uint64_t) currentthread->page_table & ~0xFFF;
        //__asm__ volatile("mov %0, %%cr3"::"r"(pml4));
        copy_page(((struct pml4t *)((uint64_t)currentthread->page_table + KERNBASE)), (uint64_t) arg1);
    }
    else {
        if (pml4t_index == 511) {
            kprintf("\nIllegal access to kernel space. This process will now close. [%x]\n", arg1);
            //map_address((uint64_t) arg1, (uint64_t) ((uint64_t) arg1 - KERNBASE));
            on_completion_pointer();
        }
        else {
            struct vm_area_struct * vmas = currentthread->mm->list_of_vmas, *vma_stack = NULL, *vma_heap = NULL;
            /*for (int i = 0; i < currentthread->mm->num_of_vmas; i++) {
                if (vmas[i].vma_type == STACK) {
                    kprintf("Stack Found");
                }
            }*/
            while (vmas != NULL) {
                if (vmas->vma_type == STACK) {
                    //kprintf("Stack Found");
                    vma_stack = vmas;
                    break;
                }
                vmas = vmas->next;
            }
            //int found = 0;
            vmas = currentthread->mm->list_of_vmas;
            while (vmas != NULL) {
                if (vmas->vma_type == HEAP) {
                    if ((uint64_t)arg1 >= vmas->vma_start && (uint64_t)arg1 <= vmas->vma_end) {
                        vma_heap = vmas;
                        //found = 1;
                        break;
                    }
                }
                vmas = vmas->next;
            }

            if (vma_stack != NULL && (uint64_t)arg1 >= vma_stack->vma_start && (uint64_t)arg1 <= vma_stack->vma_end) {
                map_user_address((uint64_t) arg1, (uint64_t) arg1 - USERBASE - 0x1000000000, 4096, (struct pml4t *)((uint64_t)currentthread->page_table+KERNBASE), 7);
            }
            else if (vma_heap != NULL && (uint64_t)arg1 >= vma_heap->vma_start && (uint64_t)arg1 <= vma_heap->vma_end) {
                map_user_address((uint64_t) arg1, (uint64_t) arg1 - USERBASE, 4096, (struct pml4t *)((uint64_t)currentthread->page_table+KERNBASE), 7);
            }
            else if(vma_stack != NULL && (uint64_t)arg1 < vma_stack->vma_start) {
                map_user_address((uint64_t) arg1, (uint64_t) arg1 - USERBASE - 0x1000000000, 4096, (struct pml4t *)((uint64_t)currentthread->page_table+KERNBASE), 7);
                vma_stack->vma_start = vma_stack->vma_start - 4096;
            }
            else {
                map_user_address((uint64_t) arg1, (uint64_t) arg1 - USERBASE, 4096, (struct pml4t *)((uint64_t)currentthread->page_table+KERNBASE), 7);
            }
            /*if (vma_stack != NULL && (uint64_t)arg1 < vma_stack->vma_start) {
                kprintf("Need to increase Stack");
                //uint64_t *stack = bump_user(4096);
            }*/
            /*__asm__ volatile ( "popq %r9 ");
            __asm__ volatile ( "popq %r9 ");
            __asm__ volatile ( "popq %r9 ");*/
            //__asm__ volatile ( "addq $24 ,%rsp ");

        }

    }
    //__asm__ volatile ( "popq %r9 ");
    //__asm__ volatile ( "popq %r9 ");
    __asm__ volatile ( "popq %r9 ");
    __asm__ volatile ( "popq %r8 ");
    __asm__ volatile ( "popq %rsi ");
    __asm__ volatile ( "popq %rbp ");
    __asm__ volatile ( "popq %rdx ");
    __asm__ volatile ( "popq %rcx ");
    __asm__ volatile ( "popq %rbx ");
    __asm__ volatile ( "popq %rax ");
    __asm__ volatile ( "popq %rdi ");
    __asm__ volatile ( "addq $32 ,%rsp ");
    __asm__( "\t movq %rax,%r9\n");
    __asm__ volatile ( "out %0, %1" : : "a"(0x20), "Nd"(0x20) );
    __asm__( "\t movq %r9,%rax\n");
    __asm__( "\tiretq\n");
};

/*
 * Function:  init_idt
 * --------------------
 * Function to create the interrupt descriptor teble
 * The function sets the address of the handler for each type of
 * interrupt and assigns the base the table to idt_addr which used in lidt
 */
void init_idt() {

    // Exceptions 0 - 31
    for(int i = 0; i < 32; i++) {
        if (i == 14) {
            idt[i].offset_1 = (uint64_t)&interrupt4;
            idt[i].offset_2 = (uint64_t)&interrupt4 >> 16;
            idt[i].offset_3 = (uint64_t)&interrupt4 >> 32;
            idt[i].zero = 0;
            idt[i].ist = 0;
            idt[i].selector = 8;
            idt[i].type_attr = 0x8e;
            continue;
        }
        if (i == 13) {
            idt[i].offset_1 = (uint64_t)&interrupt5;
            idt[i].offset_2 = (uint64_t)&interrupt5 >> 16;
            idt[i].offset_3 = (uint64_t)&interrupt5 >> 32;
            idt[i].zero = 0;
            idt[i].ist = 0;
            idt[i].selector = 8;
            idt[i].type_attr = 0x8e;
            continue;
        }
        idt[i].offset_1 = (uint64_t)&interrupt2;
        idt[i].offset_2 = (uint64_t)&interrupt2 >> 16;
        idt[i].offset_3 = (uint64_t)&interrupt2 >> 32;
        idt[i].zero = 0;
        idt[i].ist = 0;
        idt[i].selector = 8;
        idt[i].type_attr = 0x8e;
    }

    // Timer Interrupt
    idt[32].offset_1 = (uint64_t)&interrupt0;
    idt[32].offset_2 = (uint64_t)&interrupt0 >> 16;
    idt[32].offset_3 = (uint64_t)&interrupt0 >> 32;
    idt[32].zero = 0;
    idt[32].ist = 0;
    idt[32].selector = 8;
    idt[32].type_attr = 0x8e;

    // Keyboard Interrupt
    idt[33].offset_1 = (uint64_t)&interrupt1;
    idt[33].offset_2 = (uint64_t)&interrupt1 >> 16;
    idt[33].offset_3 = (uint64_t)&interrupt1 >> 32;
    idt[33].zero = 0;
    idt[33].ist = 0;
    idt[33].selector = 8;
    idt[33].type_attr = 0x8e;

    // Interrupts 34 - 255
    for(int i = 34; i < 256; i++) {
        // syscalls
        if (i == 128) {
            idt[i].offset_1 = (uint64_t)&interrupt_syscall;
            idt[i].offset_2 = (uint64_t)&interrupt_syscall >> 16;
            idt[i].offset_3 = (uint64_t)&interrupt_syscall >> 32;
            idt[i].zero = 0;
            idt[i].ist = 0;
            idt[i].selector = 8;
            idt[i].type_attr = 0xee;
            continue;
        }
        idt[i].offset_1 = (uint64_t)&interrupt3;
        idt[i].offset_2 = (uint64_t)&interrupt3 >> 16;
        idt[i].offset_3 = (uint64_t)&interrupt3 >> 32;
        idt[i].zero = 0;
        idt[i].ist = 0;
        idt[i].selector = 8;
        idt[i].type_attr = 0x8e;
    }

    struct idtr_t idtr_addr;
    idtr_addr.addr = (uint64_t)&idt;
    idtr_addr.size = sizeof(idt);
    __asm__ ( "lidt %0" : : "m"(idtr_addr) );
}





