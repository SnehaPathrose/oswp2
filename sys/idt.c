#include <sys/kprintf.h>
#include <sys/idt.h>
#include <sys/defs.h>
#define MAX_IDT 255

struct idtr_t {
    uint16_t size;
    uint64_t addr;
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

static int scount = 0;
static int mcount = 0;
static int hcount = 0;
static long mscount = 0;
//static long scount = 0;

void interrupt1() {

    register char *temp2,*temp1;
    int intvalue,i=0,rem[2];
    rem[0]=0+'0';
    rem[1]=0+'0';
    //*temp1 = 48;
    //for(temp2 = (char*)0xb8001; temp2 < (char*)0xb8000+160*25; temp2 += 2) *temp2 = 7 /* white */;

    //temp2 = (char*)0xb8f00;
    for(
            temp1 = "Time since boot: ", temp2 = (char*)(0xb8f00+100);
            *temp1;
            temp1 += 1, temp2 += 2
            ) *temp2 = *temp1;
    mscount++;
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
        //scount++;
        scount++;
        mscount = 0;
    }
    intvalue=hcount;
    while(intvalue!=0)
    {
        rem[i++]=intvalue%10+'0';
        intvalue=intvalue/10;
    }
    for(int j=1;j>=0;temp2+=2,j--)
        *temp2=rem[j];
    *temp2=':';
    temp2+=2;
    intvalue=mcount;
    rem[0]=0+'0';
    rem[1]=0+'0';
    i=0;
    while(intvalue!=0)
    {
        rem[i++]=intvalue%10+'0';
        intvalue=intvalue/10;
    }
    for(int j=1;j>=0;temp2+=2,j--)
        *temp2=rem[j];
    *temp2=':';
    temp2+=2;
    intvalue=scount;
    rem[0]=0+'0';
    rem[1]=0+'0';
    i=0;
    while(intvalue!=0)
    {
        rem[i++]=intvalue%10+'0';
        intvalue=intvalue/10;
    }
    for(int j=1;j>=0;temp2+=2,j--)
        *temp2=rem[j];
    __asm__ volatile ( "out %0, %1" : : "a"(0x20), "Nd"(0x20) );
    __asm__( "\tiretq\n");
};

const static int scantochar[100] ={'\0','\0','1','2','3','4','5','6','7','8','9','0','\0','\0','\0','\0',
                             'q','w','e','r','t','y','u','i','o','p','\0','\0','\0','\0',
                             'a','s','d','f','g','h','j','k','l','\0','\0','\0','\0','\0','z','x',
                             'c','v','b','n','m' };
const static int scantoUchar[100] ={'\0','\0','!','@','#','$','%','^','&','*','(',')','\0','\0','\0','\0',
                                   'Q','W','E','R','T','Y','U','I','O','P','\0','\0','\0','\0',
                                   'A','S','D','F','G','H','J','K','L','\0','\0','\0','\0','\0','Z','X',
                                   'C','V','B','N','M' };
static int controlflag=0x00;
void keyinterrupt() {
    // register char *temp2,*temp1;
    //unsigned long arg1;

    register char *temp2;
    temp2 = (char*) (0xb8f00-100);
    uint8_t a2;
    int bc;
    __asm__ volatile ( "inb %1, %0": "=a"(a2): "Nd"(0x60) );
    // a1 = inb(0x64);
    //__asm__("mov %%rax, %0\n": "=a"(a3));

    if ((a2 & 0x80) == 0)
    {
        if (a2==58) //caps lock
        {
            if (controlflag & 0x01)
            {
                controlflag=controlflag & 0xfe;
            }
            else
                controlflag=controlflag | 0x01;
        }
        else
        if (a2==42 || a2==54) //shift
        {
            controlflag=controlflag | 0x02;
        }
        else
        if (a2==29) // control
        {
                controlflag=controlflag | 0x04;
        }
        else {
            if (controlflag & 0x01) //caps lock
            {
                bc = a2;
                *temp2 = scantoUchar[bc];
            }
            else
            if (controlflag & 0x02) //shift
            {
                bc = a2;
                *temp2 = scantoUchar[bc];
                controlflag=controlflag & 0xfd;
            }
            else
            if (controlflag & 0x04) //ctrl
            {
                bc = a2;
                if (a2 == 45 || a2 == 46) {
                    *temp2 = '^';
                    temp2 += 2;
                    *temp2=scantoUchar[bc];
                }
                controlflag=controlflag & 0xfb;
            }
            // if (a2 == 0x10)
            else {
                bc = a2;
                *temp2 = scantochar[bc];
            }

            // if (a4==0);
            //if (scantochar == 0);
        }
    }

    //if (a2 == 0x10 || a2 == 16)
    //__asm__( "\tpushad\n");
    __asm__("\tpush %rax\n");
    __asm__("\tpush %rcx\n");
    __asm__("\tpush %rdx\n");
    __asm__("\tpush %rsi\n");
    __asm__("\tpush %rdi\n");
    __asm__("\tpush %r8\n");
    __asm__("\tpush %r9\n");
    kprintf("Hello[%d]", 5);
    __asm__("\tpop %r9\n");
    __asm__("\tpop %r8\n");
    __asm__("\tpop %rdi\n");
    __asm__("\tpop %rsi\n");
    __asm__("\tpop %rdx\n");
    __asm__("\tpop %rcx\n");
    __asm__("\tpop %rax\n");
    __asm__("\tpop %r11\n");
    //__asm__( "\tpopad\n");
    __asm__ volatile ( "out %0, %1" : : "a"(0x20), "Nd"(0x20) );
    __asm__( "\tiretq\n");
};

void interrupt2() {

    __asm__ volatile ( "out %0, %1" : : "a"(0x20), "Nd"(0x20) );
    __asm__( "\tiretq\n");
};

void interrupt3() {
    //kprintf("Here6");
    __asm__ volatile ( "out %0, %1" : : "a"(0x20), "Nd"(0x20) );
    __asm__( "\tiretq\n");
};

struct idtr_item idt[MAX_IDT];

void init_idt() {

    for(int i = 0; i < 32; i++) {
        idt[i].offset_1 = (uint64_t)&interrupt2;
        idt[i].offset_2 = (uint64_t)&interrupt2 >> 16;
        idt[i].offset_3 = (uint64_t)&interrupt2 >> 32;
        idt[i].zero = 0;
        idt[i].ist = 0;
        idt[i].selector = 8;
        idt[i].type_attr = 0x8e;
    }

    idt[32].offset_1 = (uint64_t)&interrupt1;
    idt[32].offset_2 = (uint64_t)&interrupt1 >> 16;
    idt[32].offset_3 = (uint64_t)&interrupt1 >> 32;
    idt[32].zero = 0;
    idt[32].ist = 0;
    idt[32].selector = 8;
    idt[32].type_attr = 0x8e;

    idt[33].offset_1 = (uint64_t)&keyinterrupt;
    idt[33].offset_2 = (uint64_t)&keyinterrupt >> 16;
    idt[33].offset_3 = (uint64_t)&keyinterrupt >> 32;
    idt[33].zero = 0;
    idt[33].ist = 0;
    idt[33].selector = 8;
    idt[33].type_attr = 0x8e;

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


