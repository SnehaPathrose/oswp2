#include <sys/defs.h>
#include <sys/kprintf.h>
#include <sys/pci.h>
#include <sys/ahci.h>
#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	 0x96690101	// Port multiplier
#define HBA_PORT_DET_PRESENT 3
#define HBA_PORT_IPM_ACTIVE 1
#define AHCI_DEV_NULL 0
#define ATA_CMD_READ_DMA_EX 0x25
#define ATA_CMD_WRITE_DMA_EX 0x35
/*#define AHCI_DEV_SATAPI 4
#define AHCI_DEV_SEMB 2
#define */

#define	AHCI_BASE	0x400000	// 4M
#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08

void memset(void* s, int num, int size)
{
    for (int i = 0; i < size; i++) {
        *(int*)s = num;
        s++;
    }
}

// Find a free command list slot
int find_cmdslot(hba_port_t *port)
{
    // If not set in SACT and CI, the slot is free
    uint32_t slots = (port->sact | port->ci);
    for (int i=0; i<32; i++)
    {
        if ((slots&1) == 0)
            return i;
        slots >>= 1;
    }
    kprintf("Cannot find free command list entry\n");
    return -1;
}

int read(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint32_t *buf)
{
    port->is_rwc = (uint32_t)-1;		// Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;

    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint32_t);	// Command FIS size
    cmdheader->w = 0;		// Read from device
    cmdheader->prdtl = (uint32_t)((count-1)>>4) + 1;	// PRDT entries count

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)cmdheader->ctba;
    memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) +
                      (cmdheader->prdtl-1)*sizeof(hba_prdt_entry_t));

    // 8K bytes (16 sectors) per PRDT
    int i = 0;
    for (i=0; i<cmdheader->prdtl-1; i++)
    {
        cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
        cmdtbl->prdt_entry[i].dbc = 8*1024;	// 8K bytes
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4*1024;	// 4K words
        count -= 16;	// 16 sectors
    }
    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
    cmdtbl->prdt_entry[i].dbc = count<<9;	// 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 1;

    // Setup command
    fis_reg_h2d_t *cmdfis = (fis_reg_h2d_t*)&cmdtbl->cfis;
    //fis_type_t t;
    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;	// Command
    cmdfis->command = ATA_CMD_READ_DMA_EX;

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl>>8);
    cmdfis->lba2 = (uint8_t)(startl>>16);
    cmdfis->device = 1<<6;	// LBA mode

    cmdfis->lba3 = (uint8_t)(startl>>24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth>>8);

    cmdfis->count = count;
    //cmdfis->counth = HIBYTE(count);

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        kprintf("Port is hung\n");
        return 0;
    }

    port->ci = 1<<slot;	// Issue command

    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1<<slot)) == 0)
            break;
        if (port->is_rwc & HBA_PxIS_TFES)	// Task file error
        {
            kprintf("Read disk error\n");
            return 0;
        }
    }

    // Check again
    if (port->is_rwc & HBA_PxIS_TFES)
    {
        kprintf("Read disk error\n");
        return 0;
    }

    return 1;
}


int write(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint32_t *buf)
{
    port->is_rwc = (uint32_t)-1;		// Clear pending interrupt bits
    int spin = 0; // Spin lock timeout counter
    int slot = find_cmdslot(port);
    if (slot == -1)
        return 0;

    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    cmdheader += slot;
    cmdheader->cfl = sizeof(fis_reg_h2d_t)/sizeof(uint32_t);	// Command FIS size
    cmdheader->w = 1;		// Write to device
    cmdheader->prdtl = (uint32_t)((count-1)>>4) + 1;	// PRDT entries count

    hba_cmd_tbl_t *cmdtbl = (hba_cmd_tbl_t*)cmdheader->ctba;
    memset(cmdtbl, 0, sizeof(hba_cmd_tbl_t) +
                      (cmdheader->prdtl-1)*sizeof(hba_prdt_entry_t));

    // 8K bytes (16 sectors) per PRDT
    int i = 0;
    for (i=0; i<cmdheader->prdtl-1; i++)
    {
        cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
        cmdtbl->prdt_entry[i].dbc = 8*1024;	// 8K bytes
        cmdtbl->prdt_entry[i].i = 1;
        buf += 4*1024;	// 4K words
        count -= 16;	// 16 sectors
    }
    // Last entry
    cmdtbl->prdt_entry[i].dba = (uint64_t)buf;
    cmdtbl->prdt_entry[i].dbc = count<<9;	// 512 bytes per sector
    cmdtbl->prdt_entry[i].i = 1;

    // Setup command
    fis_reg_h2d_t *cmdfis = (fis_reg_h2d_t*)&cmdtbl->cfis;
    //fis_type_t t;
    cmdfis->fis_type = FIS_TYPE_REG_H2D;
    cmdfis->c = 1;	// Command
    cmdfis->command = ATA_CMD_WRITE_DMA_EX;

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl>>8);
    cmdfis->lba2 = (uint8_t)(startl>>16);
    cmdfis->device = 1<<6;	// LBA mode

    cmdfis->lba3 = (uint8_t)(startl>>24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth>>8);

    cmdfis->count = count;
    //cmdfis->counth = HIBYTE(count);

    // The below loop waits until the port is no longer busy before issuing a new command
    while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
    {
        spin++;
    }
    if (spin == 1000000)
    {
        kprintf("Port is hung\n");
        return 0;
    }

    port->ci = 1<<slot;	// Issue command

    // Wait for completion
    while (1)
    {
        // In some longer duration reads, it may be helpful to spin on the DPS bit
        // in the PxIS port field as well (1 << 5)
        if ((port->ci & (1<<slot)) == 0)
            break;
        if (port->is_rwc & HBA_PxIS_TFES)	// Task file error
        {
            kprintf("Read disk error\n");
            return 0;
        }
    }

    // Check again
    if (port->is_rwc & HBA_PxIS_TFES)
    {
        kprintf("Read disk error\n");
        return 0;
    }

    return 1;
}


// Start command engine
void start_cmd(hba_port_t *port)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);

    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

// Stop command engine
void stop_cmd(hba_port_t *port)
{
    // Clear ST (bit0)
    port->cmd &= ~HBA_PxCMD_ST;

    // Wait until FR (bit14), CR (bit15) are cleared
    while(1)
    {
        if (port->cmd & HBA_PxCMD_FR)
            continue;
        if (port->cmd & HBA_PxCMD_CR)
            continue;
        break;
    }
    kprintf("Here at last\n");
    // Clear FRE (bit4)
    port->cmd &= ~HBA_PxCMD_FRE;
}



void port_rebase(hba_port_t *port, int portno)
{
    stop_cmd(port);	// Stop command engine

    // Command list offset: 1K*portno
    // Command list entry size = 32
    // Command list entry maxim count = 32
    // Command list maxim size = 32*32 = 1K per port
    port->clb = AHCI_BASE + (portno<<10);
    port->clb = port->clb & 0x00000000ffffffff;
    memset((void*)(port->clb), 0, 1024);

    // FIS offset: 32K+256*portno
    // FIS entry size = 256 bytes per port
    port->fb = AHCI_BASE + (32<<10) + (portno<<8);
    port->fb = port->fb  & 0x00000000ffffffff;
    memset((void*)(port->fb), 0, 256);

    // Command table offset: 40K + 8K*portno
    // Command table size = 256*32 = 8K per port
    hba_cmd_header_t *cmdheader = (hba_cmd_header_t*)port->clb;
    for (int i=0; i<32; i++)
    {
        cmdheader[i].prdtl = 8;	// 8 prdt entries per command table
        // 256 bytes per command table, 64+16+48+16*8
        // Command table offset: 40K + 8K*portno + cmdheader_index*256
        cmdheader[i].ctba = AHCI_BASE + (40<<10) + (portno<<13) + (i<<8);
        cmdheader[i].ctba =cmdheader[i].ctba & 0x00000000ffffffff;
        memset((void*)cmdheader[i].ctba, 0, 256);
    }

    start_cmd(port);	// Start command engine
}



static int check_type(hba_port_t *port)
{
    uint32_t ssts = port->ssts;

    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;
    //kprintf("value of det and ipm are %d %d\n",det,ipm);
    //kprintf("value of sig is %x\n",port->sig);
    if (det != HBA_PORT_DET_PRESENT)	// Check drive status
        return AHCI_DEV_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
        return AHCI_DEV_NULL;

    switch (port->sig)
    {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        default:
            return AHCI_DEV_SATA;
    }
}

void probe_port(hba_mem_t *abar)
{
    // Search disk in impelemented ports
    uint32_t pi = abar->pi;
    int i = 0;
    while (i<32)
    {
        if (pi & 1)
        {
            //kprintf("probing port %d",i);
            int dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                kprintf("SATA drive found at port %d\n", i);
                abar->ghc = 0x80000003;
                port_rebase(&abar->ports[i], i);
                uint32_t buf = 20;
                write(&abar->ports[i], 1, 0, 1, &buf);
                kprintf("Value of buf %x\n", buf);

                uint32_t readbuf = 0;
                read(&abar->ports[i], 1, 0, 1, &readbuf);
                kprintf("Value of buf %x\n", readbuf);
                break;
            }
            else if (dt == AHCI_DEV_SATAPI)
            {
                kprintf("SATAPI drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_SEMB)
            {
                kprintf("SEMB drive found at port %d\n", i);
            }
            else if (dt == AHCI_DEV_PM)
            {
                kprintf("PM drive found at port %d\n", i);
            }
            /*else
            {
                kprintf("No drive found at port %d\n", i);
            }*/
        }

        pi >>= 1;
        i ++;
    }
}

void checkbus()
{
    uint32_t address;
    uint32_t address1;
    uint32_t address2;
    uint32_t address3;
    //uint64_t address4;
    //hba_mem_t ahci_memory;
    uint32_t bus = 0;
    uint32_t device_no = 0;
    uint32_t function = 0;
    uint32_t tmp = 0;
    uint32_t tmp1 = 0;
    uint32_t tmp2 = 0;
    uint32_t tmp3 = 0;
    uint32_t tmp4 = 0;
    //uint64_t tmp5 = 0;
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
                __asm__ volatile ( "outl %0, %1" : : "a"(address2), "Nd"((uint16_t)0xCF8));
                __asm__ volatile ( "inl %1, %0": "=a"(tmp2): "Nd"((uint16_t)(0xCFC)));
                tmp2 = (uint8_t)((tmp2 >> ((0x03) * 8)) & 0xffff);


                if (tmp != 0xffffffff && tmp2 == 0x01 && tmp3 == 0x06) {
                    kprintf("Vendor: %x\n", (uint16_t)tmp);
                    kprintf("Device: %x\n", tmp1);
                    kprintf("Class Id: %x\n", tmp2);
                    kprintf("Sub Class Id: %x\n", tmp3);
                    kprintf("Address Class: %x\n", address2);
                    kprintf("Address Sub Class: %x\n", address3);
                    kprintf("Address1: %x\n", address1);
                    kprintf("Address: %x\n", address);

                    address2 = (uint32_t)((bus << 16) | (device_no << 11) |
                                          (function << 8) | (0x24 & 0xfc) | ((uint32_t) 0x80000000));
                    kprintf("address of bar[5] is %x\n",address2);
                    __asm__ volatile ( "outl %0, %1" : : "a"(address2), "Nd"((uint16_t) 0xCF8));
                    __asm__ volatile ( "inl %1, %0": "=a"(tmp4): "Nd"((uint16_t)(0xCFC)));
                    //ahci_memory = (hba_mem_t)((ahci_memory >> ((0x03) * 8)) & 0xffff);
                    kprintf("value of bar[5] is %x\n",tmp4);
                    //uint32_t ab = (uint64_t)&ahci_memory & 0xffffffff;
                    __asm__ volatile ( "outl %0, %1" : : "a"(address2), "Nd"((uint16_t) 0xCF8));
                    //uint32_t ab = (uint32_t)ahci_memory;
                    __asm__ volatile ( "outl %0, %1": : "a"(0x8000000), "Nd"((uint16_t)(0xCFC)));

                    __asm__ volatile ( "outl %0, %1" : : "a"(address2), "Nd"((uint16_t) 0xCF8));
                    __asm__ volatile ( "inl %1, %0": "=a"(tmp4): "Nd"((uint16_t)(0xCFC)));
                    //hba_mem_t* ahci_memory1;
                    //__asm__("\t mov %0,%%eax\n" : "=m"(tmp4));
                    //__asm__("\t mov %%eax,%0\n" : "=m"(ahci_memory1));
                    //ahci_memory.cap = ahci_memory.cap;
                    uint64_t address4 = tmp4 & 0xffffffffffffffff;
                    hba_mem_t* ahci_memory1 = (hba_mem_t *)address4;
                    kprintf("value of abar is %x\n",ahci_memory1);
                    kprintf("value of abar cap is %x\n",ahci_memory1->cap);
                    kprintf("value of abar pi is %x\n",ahci_memory1->pi);
                    probe_port(ahci_memory1);
                    //__asm__("\t mov %%rsi,%0\n" : "=m"(tmp5));
                    /*address4 = tmp4 & 0xffffffff80000000;
                    ahci_memory = (hba_mem_t *)address4;
                    kprintf("value of abar is %x\n",ahci_memory);
                    kprintf("value of abar cap is %x\n",ahci_memory->cap);
                    kprintf("value of abar pi is %x\n",ahci_memory->pi);*/
                    /*address4 = tmp4 & 0xfffffff0;
                    //ahci_memory = (hba_mem_t *)address4;
                    //kprintf("value of abar is %x",ahci_memory);
                    //ahci_memory = address;
                    address2 = (uint32_t)((bus << 16) | (device_no << 11) |
                                          (function << 8) | (0x24 & 0xfc) | ((uint32_t) 0x80000000));
                    __asm__ volatile ( "outl %0, %1" : : "a"(address2), "Nd"((uint16_t) 0xCF8));
                    __asm__ volatile ( "outl %0, %1": : "a"(0xFEE0B000), "Nd"((uint16_t)(0xCFC)));
                    //read
                    address2 = (uint32_t)((bus << 16) | (device_no << 11) |
                                          (function << 8) | (0x24 & 0xfc) | ((uint32_t) 0x80000000));
                    __asm__ volatile ( "outl %0, %1" : : "a"(address2), "Nd"((uint16_t) 0xCF8));
                    __asm__ volatile ( "inl %1, %0": "=a"(tmp4): "Nd"((uint16_t)(0xCFC)));
                    address4 = tmp4 & 0xfffffff0;
                    ahci_memory = (hba_mem_t *)(address4);
                    kprintf("value of abar is %x\n",ahci_memory);
                    kprintf("value of abar cap is %x\n",ahci_memory->cap);
                    probe_port(ahci_memory);*/
                }
            }
        }
    }
    kprintf("Finished!!");
}