#include <sys/defs.h>
#include <sys/io.h>
#include <sys/pci.h>
#include <sys/ahci.h>
#include <sys/klibc.h>

/*
 * Function:  find_cmdslot 
 * --------------------
 * Find a free command list slot
 */
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

/*
 * Function:  read 
 * --------------------
 * Read from a device from AHCI <port> from <starth:startl> to <count> size
 * from <buf>
 */
int read(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint32_t *buf)
{
    port->is_rwc = (uint32_t) - 1;		// Clear pending interrupt bits
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
    cmdfis->pmport = 0;
    cmdfis->c = 1;	// Command
    cmdfis->command = ATA_CMD_READ_DMA_EX;
    cmdfis->featurel = 0;

    cmdfis->lba0 = (uint8_t)startl;
    cmdfis->lba1 = (uint8_t)(startl>>8);
    cmdfis->lba2 = (uint8_t)(startl>>16);
    cmdfis->device = 1<<6;	// LBA mode

    cmdfis->lba3 = (uint8_t)(startl>>24);
    cmdfis->lba4 = (uint8_t)starth;
    cmdfis->lba5 = (uint8_t)(starth>>8);

    cmdfis->count = count;

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

/*
 * Function:  write 
 * --------------------
 * Write to a device from AHCI <port> from <starth:startl> to <count> size
 * and store it in <buf>
 */
int write_to_disk(hba_port_t *port, uint32_t startl, uint32_t starth, uint32_t count, uint32_t *buf)
{
    hba_cmd_tbl_t *cmdtbl;
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

    cmdtbl = (hba_cmd_tbl_t*)cmdheader->ctba;
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

/*
 * Function:  start_cmd 
 * --------------------
 * Start command engine
 */
void start_cmd(hba_port_t *port)
{
    // Wait until CR (bit15) is cleared
    while (port->cmd & HBA_PxCMD_CR);

    // Set FRE (bit4) and ST (bit0)
    port->cmd |= HBA_PxCMD_FRE;
    port->cmd |= HBA_PxCMD_ST;
}

/*
 * Function:  stop_cmd 
 * --------------------
 * Stop command engine
 */
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
    // Clear FRE (bit4)
    port->cmd &= ~HBA_PxCMD_FRE;
}

/*
 * Function:  port_rebase 
 * --------------------
 * Rebase AHCI port to system physical memory
 */
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
    
    port->sctl |= 0x301;
    for (int i=0;i<100000000;i++);
    port->sctl &= 0xfffffffe;
    for (int i=0;i<100000000;i++);
    
    port->cmd |= (HBA_PxCMD_SUD | HBA_PxCMD_POD | HBA_PxCMD_ICC);
    for (int i=0;i<100000000;i++);
    
    port->serr_rwc = 0xffffffff;
    for (int i=0;i<100000000;i++);
    port->is_rwc = 0xffffffff;
    for (int i=0;i<100000000;i++);
    start_cmd(port);	// Start command engine
}

/*
 * Function:  check_type 
 * --------------------
 * Check type of device
 */
static int check_type(hba_port_t *port)
{
    switch (port->sig)
    {
        case SATA_SIG_ATAPI:
            return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEV_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEV_PM;
        case SATA_SIG_ATA:
            return AHCI_DEV_SATA;
    }
    return 0;
}

/*
 * Function:  probe_port 
 * --------------------
 * Probe each port from ABAR to find the SATA port
 */
void probe_port(hba_mem_t *abar)
{
    // Search disk in impelemented ports
    uint32_t pi = abar->pi;
    uint32_t *readbuf = (uint32_t *)0x210400;
    uint32_t *writebuf = (uint32_t *)0x210000;
    int i = 0;
    while (i < 32)
    {
        if (pi & 1)
        {
            int dt = check_type(&abar->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                kprintf("SATA found at %d ", i);
                //abar->ghc |= (HBA_GHC_AE | HBA_GHC_IE | HBA_GHC_HR);
                abar->ghc |= HBA_GHC_HR;
                abar->ghc |= HBA_GHC_AE;
                abar->ghc |= HBA_GHC_IE;
                port_rebase(&abar->ports[i], i);
                
                //WRITE
                for (int b = 0;b < 100;b++)
                {
                    memset(writebuf, b, 1024*sizeof(uint32_t));
                    write_to_disk(&abar->ports[i], 8 * b, 0, 8, writebuf);
                }
               
                //READ
                for (int r =0;r < 100;r++)
                {
                    memset(readbuf, 0, 1024*sizeof(uint32_t));
                    read(&abar->ports[i], 8 * r, 0, 8, readbuf);
                    //print from each block
                    kprintf("%x", *readbuf);
                }
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
        }

        pi >>= 1;
        i++;
    }
}

/*
 * Function:  read_write_to_device 
 * --------------------
 * Read/write config to device
 */
uint32_t read_write_to_device(int bus, int device_no, int function, uint8_t offset)
{
    uint32_t address, ret_val;
    address = (uint32_t)((bus << 16) | (device_no << 11) |
                                     (function << 8) | (offset & 0xfc) | ((uint32_t) 0x80000000));
    __asm__ volatile ( "outl %0, %1" : : "a"(address), "Nd"((uint16_t) 0xCF8));
    __asm__ volatile ( "inl %1, %0": "=a"(ret_val): "Nd"((uint16_t)(0xCFC)));
    return ret_val;
}

/*
 * Function:  checkbus 
 * --------------------
 * Check all buses to get the value of ABAR
 */
void checkbus()
{
    uint32_t abar_address, abar;
    //uint32_t mask;
    int bus, device_no, function;
    uint32_t vendor_id = 0 , device_id = 0, sub_class_id = 0, class_id = 0;
    
    for (bus = 0; bus < 256; bus++) {
        for (device_no = 0; device_no < 32; device_no++) {
            for (function = 0; function < 8; function++) {
                // Reading Vendor Id
                vendor_id = read_write_to_device(bus, device_no, function, 0);
                
                // Reading Device Id
                device_id = read_write_to_device(bus, device_no, function, 0x02);
                device_id = (uint16_t)((device_id >> (0x02 & 2) * 8) & 0xffff);

                // Reading Sub-Class Id
                sub_class_id = read_write_to_device(bus, device_no, function, 0x0a);
                sub_class_id = (uint8_t)((sub_class_id >> ((0x02) * 8)) & 0xffff);

                // Reading Class Id
                class_id = read_write_to_device(bus, device_no, function, 0x0b);
                class_id = (uint8_t)((class_id >> ((0x03) * 8)) & 0xffff);


                if (vendor_id != 0xffffffff && class_id == 0x01 && sub_class_id == 0x06) {
                    kprintf("AHCI controller found at bus:%d, device: %d and function: %d\n", bus, device_no, function);
                    kprintf("Vendor: %x\n", (uint16_t)vendor_id);
                    kprintf("Device: %x\n", device_id);
                    kprintf("Class Id: %x\n", class_id);
                    kprintf("Sub Class Id: %x\n", sub_class_id);
                    
                    //Remapping ABAR
                    abar_address = (uint32_t)((bus << 16) | (device_no << 11) |
                                          (function << 8) | (0x24 & 0xfc) | ((uint32_t) 0x80000000));
                    __asm__ volatile ( "outl %0, %1" : : "a"(abar_address), "Nd"((uint16_t) 0xCF8));
                    __asm__ volatile ( "outl %0, %1": : "a"(ABAR_REMAP), "Nd"((uint16_t)(0xCFC)));

                    __asm__ volatile ( "outl %0, %1" : : "a"(abar_address), "Nd"((uint16_t) 0xCF8));
                    __asm__ volatile ( "inl %1, %0": "=a"(abar): "Nd"((uint16_t)(0xCFC)));
                    uint64_t abar_64 = abar & 0xffffffffffffffff;
                    hba_mem_t* ahci_memory = (hba_mem_t *)abar_64;
                    
                    //check for port implementation
                    probe_port(ahci_memory);
                }
            }
        }
    }
    
}