#include <sys/defs.h>


void init_pic(int offset1)
{
    unsigned char a1, a5;
    unsigned short a2 = 0x11;

    //a1 = inb(0x21);

    __asm__ volatile ( "inb %1, %0": "=a"(a1): "Nd"(0x21) );
    __asm__ volatile ( "inb %1, %0": "=a"(a5): "Nd"(0xa1) );

    __asm__ volatile ( "outb %0, $0x20" : : "a"((uint8_t)a2));
    __asm__ volatile ( "outb %%al, $0x80" : : "a"(0) );

    __asm__ volatile ( "outb %0, %1" : : "a"((uint8_t)0x11), "Nd"(0xa0) );
    __asm__ volatile ( "outb %%al, $0x80" : : "a"(0) );

    __asm__ volatile ( "outb %0, %1" : : "a"((uint8_t)0x20), "Nd"(0x21) );
    __asm__ volatile ( "outb %%al, $0x80" : : "a"(0) );

    __asm__ volatile ( "outb %0, %1" : : "a"((uint8_t)0x34), "Nd"(0xa1) );
    __asm__ volatile ( "outb %%al, $0x80" : : "a"(0) );

    __asm__ volatile ( "outb %0, %1" : : "a"((uint8_t)0x04), "Nd"(0x21) );
    __asm__ volatile ( "outb %%al, $0x80" : : "a"(0) );

    __asm__ volatile ( "outb %0, %1" : : "a"((uint8_t)0x02), "Nd"(0xa1) );
    __asm__ volatile ( "outb %%al, $0x80" : : "a"(0) );

    __asm__ volatile ( "outb %0, %1" : : "a"((uint8_t)0x01), "Nd"(0x21) );
    __asm__ volatile ( "outb %%al, $0x80" : : "a"(0) );

    __asm__ volatile ( "outb %0, %1" : : "a"((uint8_t)0x01), "Nd"(0xa1) );
    __asm__ volatile ( "outb %%al, $0x80" : : "a"(0) );


    __asm__ volatile ( "outb %0, %1" : : "a"(a1), "Nd"(0x21) );
    __asm__ volatile ( "outb %0, %1" : : "a"(a5), "Nd"(0xa1) );

    __asm__ volatile ( "outb %0, %1" : : "a"((uint8_t)0xfc), "Nd"(0x21) );
}
