#include <sys/defs.h>

/*
 * Function:  init_pic 
 * --------------------
 * Initializes the PIC by setting the offset for master and slave 
 * and sets it's mask
 */
void init_pic()
{
    unsigned char pic1mask, pic2mask;

    __asm__ volatile ( "inb %1, %0": "=a"(pic1mask): "Nd"(0x21) );
    __asm__ volatile ( "inb %1, %0": "=a"(pic2mask): "Nd"(0xa1) );

    __asm__ volatile ( "outb %0, $0x20" : : "a"((uint8_t)0x11));
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


    __asm__ volatile ( "outb %0, %1" : : "a"(pic1mask), "Nd"(0x21) );
    __asm__ volatile ( "outb %0, %1" : : "a"(pic2mask), "Nd"(0xa1) );

    __asm__ volatile ( "outb %0, %1" : : "a"((uint8_t)0xfd), "Nd"(0x21) );
    // __asm__ volatile ( "outb %0, %1" : : "a"((uint8_t)0xdf), "Nd"(0xa1) );
}

