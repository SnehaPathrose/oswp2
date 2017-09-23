#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/pci.h>
#include <sys/ahci.h>

void checkbus()
{
    uint32_t address;
    uint32_t address1;
    uint32_t address2;
    uint32_t address3;
    hba_mem_t* ahci_memory;
    uint32_t bus = 0;
    uint32_t device_no = 0;
    uint32_t function = 0;
    uint32_t tmp = 0;
    uint32_t tmp1 = 0;
    uint32_t tmp2 = 0;
    uint32_t tmp3 = 0;
    for (bus = 0; bus < 256; bus++) {
        for (device_no = 0; device_no < 32; device_no++) {
            for (function = 0; function < 8; function++) {
                address = (uint32_t)((bus << 16) | (device_no << 11) |
                                     (function << 8) | (0 & 0xfc) | ((uint32_t) 0x80000000));
                __asm__ volatile ( "outl %0, %1" : : "a"(address), "Nd"((uint16_t) 0xCF8));
                __asm__ volatile ( "inl %1, %0": "=a"(tmp): "Nd"((uint16_t)(0xCFC)));


                address1 = (uint32_t)((bus << 16) | (device_no << 11) |
                                     (function << 8) | (0x02 & 0xfc) | ((uint32_t) 0x80000000));
                __asm__ volatile ( "outl %0, %1" : : "a"(address1), "Nd"((uint16_t) 0xCF8));
                __asm__ volatile ( "inl %1, %0": "=a"(tmp1): "Nd"((uint16_t)(0xCFC)));
                tmp1 = (uint16_t)((tmp1 >> (0x02 & 2) * 8) & 0xffff);

                //subclass
                address3 = (uint32_t)((bus << 16) | (device_no << 11) |
                                     (function << 8) | (0x0a & 0xfc) | ((uint32_t) 0x80000000));
                __asm__ volatile ( "outl %0, %1" : : "a"(address3), "Nd"((uint16_t) 0xCF8));
                __asm__ volatile ( "inl %1, %0": "=a"(tmp3): "Nd"((uint16_t)(0xCFC)));
                tmp3 = (uint8_t)((tmp3 >> ((0x02) * 8)) & 0xffff);

                //class
                address2 = (uint32_t)((bus << 16) | (device_no << 11) |
                                     (function << 8) | (0x0b & 0xfc) | ((uint32_t) 0x80000000));
                __asm__ volatile ( "outl %0, %1" : : "a"(address2), "Nd"((uint16_t) 0xCF8));
                __asm__ volatile ( "inl %1, %0": "=a"(tmp2): "Nd"((uint16_t)(0xCFC)));
                tmp2 = (uint8_t)((tmp2 >> ((0x03) * 8)) & 0xffff);


                if (tmp != 0xffffffff && tmp2 == 0x01 && tmp3 == 0x06) {
                    kprintf("\n\rVendor: %x ", (uint16_t)tmp);
                    kprintf("Device: %x  ", tmp1);
                    kprintf("Class Id: %x ", tmp2);
                    kprintf("Sub Class Id: %x ", tmp3);
                    kprintf("Address Class: %x ", address2);
                    kprintf("Address Sub Class: %x ", address3);
                    kprintf("Address1: %x ", address1);
                    kprintf("Address: %x ", address);

                    address2 = (uint32_t)((bus << 16) | (device_no << 11) |
                                          (function << 8) | (0x24 & 0xfc) | ((uint32_t) 0x80000000));
                    __asm__ volatile ( "outl %0, %1" : : "a"(address2), "Nd"((uint16_t) 0xCF8));
                    __asm__ volatile ( "inl %1, %0": "=a"(ahci_memory): "Nd"((uint16_t)(0xCFC)));
                    //ahci_memory = (hba_mem_t)((ahci_memory >> ((0x03) * 8)) & 0xffff);

                    //ahci_memory = address;
                }
            }
        }
    }
    kprintf("Finished!!");
}