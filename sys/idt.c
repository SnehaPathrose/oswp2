#include <sys/kprintf.h>
#include <sys/idt.h>
#include <sys/defs.h>
#include <sys/virtualmem.h>

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
const static int scantochar[100] = { '\0','\0','1','2','3','4','5','6','7','8','9','0','\0','\0','\0','\0',
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
                                   'C','V','B','N','M' };
static int controlflag = 0x00;
static int scount = 0;   // count in seconds
static int mcount = 0;   // count in minutes
static int hcount = 0;   // count in hours
static long mscount = 0; // count in timer frequency

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

/*
 * Function:  interrupt1 
 * --------------------
 * Interrupt handler for the keyboard interrupt
 * Keeps track of the key which was pressed and writes to the screen.
 * Also keeps track of control, shift and capslock characters through control flag
 */
void interrupt1() {
    register char *temp2 = (char *)(KERNBASE + 0xb8ef0);
    uint8_t keyscancode;
    __asm__ volatile ( "inb %1, %0": "=a"(keyscancode): "Nd"(0x60) );

    if ((keyscancode & 0x80) == 0)
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
    }

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
 * Function:  init_idt
 * --------------------
 * Function to create the interrupt descriptor teble
 * The function sets the address of the handler for each type of
 * interrupt and assigns the base the table to idt_addr which used in lidt
 */
void init_idt() {

    // Exceptions 0 - 31
    for(int i = 0; i < 32; i++) {
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


