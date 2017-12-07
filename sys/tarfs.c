//
// Created by sneha pathrose on 11/19/17.
//

#include <sys/tarfs.h>
#include <sys/elf64.h>
#include <sys/klibc.h>
#include <sys/allocator.h>
#include <sys/contextswitch.h>
#include <sys/virtualmem.h>
#include <sys/io.h>
#include <sys/process.h>

uint64_t getsize(const char *in)
{

    uint64_t size = 0;
    unsigned int j;
    unsigned int count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;

}

uint64_t findfile(char *filename)
{
    struct posix_header_ustar *tarfs;
    uint64_t size;
    //kprintf("size of : %d",sizeof(struct posix_header_ustar));
    uint64_t address=(uint64_t)&_binary_tarfs_start;
    tarfs = (struct posix_header_ustar *)address;
    while(tarfs->name[0]!='\0')
    { size=getsize(tarfs->size);
        //kprintf("name: %s ",tarfs->name);
        //kprintf("size: %d",size);
        //kprintf("typeflag: %s\n",tarfs->typeflag);
        if(kstrcmp(tarfs->name,filename) == 0)
            return address;
        address += ((size/512)+1) * 512;
        tarfs = (struct posix_header_ustar *) address;
        if(size%512) {
            address += 512;
            tarfs = (struct posix_header_ustar *) address;
        }
    }
    return 0;
}

void loadelf(char *filename, struct PCB *p1)
{
    if(p1==NULL)
        p1=bump(sizeof(struct PCB));
    uint64_t elfaddress=findfile(filename) + (uint64_t)sizeof(struct posix_header_ustar);
    int i;
    uint64_t addr,phy;
    struct Elf64_Ehdr *elf=(struct Elf64_Ehdr *) elfaddress;
    //check if elf
    struct Elf64_Phdr *phdr=(struct Elf64_Phdr *) (elfaddress + elf->e_phoff);
    p1->mm = bump(sizeof(struct mm_struct));
    p1->gotoaddr = elf->e_entry;
    //kprintf("elf program hsize: %x\n",elf->e_phentsize);
    struct vm_area_struct *tempvma = NULL;
    struct vm_area_struct *vma;
    for(i=0;i<elf->e_phnum;i++)
    {
        if(phdr->p_type==1) {
            vma = bump(sizeof(struct vm_area_struct));
            vma->vma_start = phdr->p_vaddr;
            vma->vma_end = phdr->p_vaddr+phdr->p_memsz;
            vma->vma_flags=phdr->p_flags;
            vma->vma_type=OTHER;

            //vma->vma_file=NULL;
            if(phdr->p_flags == (PF_R + PF_X) )
            {
                struct file *file = (struct file *)bump(sizeof(struct file));
                vma->vma_file = file;
                vma->vma_file->file_start = (uint64_t)elf;
                vma->vma_file->vm_pgoff = phdr->p_offset;
                vma->vma_file->vm_sz = phdr->p_filesz;
                vma->vma_file->bss_size = 0;
            }
            else if(phdr->p_flags == (PF_R + PF_W) ){
                vma->vma_file = (struct file *)bump(sizeof(struct file));
                vma->vma_file->file_start = (uint64_t)elf;
                vma->vma_file->vm_pgoff = phdr->p_offset;
                vma->vma_file->vm_sz = phdr->p_filesz;
                vma->vma_file->bss_size = phdr->p_memsz - phdr->p_filesz;
            }
            uint64_t sizetocopy;
            for(addr=phdr->p_vaddr,i=0;addr<phdr->p_vaddr+phdr->p_memsz;addr+=4096,i+=4096)
            {
                phy=(uint64_t)bump_physical(4096);
                map_user_address(addr,phy,4096,(struct pml4t *)((uint64_t)p1->page_table+KERNBASE),0x07);
                if((phdr->p_memsz - i)>4096)
                   sizetocopy = 4096;
                else
                    sizetocopy = phdr->p_memsz - i;
                kmemcpychar((void *)(elf+phdr->p_offset+i),(void *)addr,sizetocopy);

            }
            flush_tlb();


            vma->next = NULL;
            if(tempvma == NULL)
            {
                p1->mm->list_of_vmas=vma;
                vma->prev=NULL;
            }
            else
            {
                vma->prev = tempvma;
            }
            p1->mm->num_of_vmas++;
            tempvma = vma;
            vma=vma->next;
        }
        phdr = (struct Elf64_Phdr *)((uint64_t)phdr+sizeof(struct Elf64_Phdr));

    }

    //create process heap
    createheap(PAGE_SIZE,p1,0x07);

    //create process stack
    uint64_t *stack = bump_user(4096);
    //kprintf("\n Value of bump: %x", stack);
    stack = stack + 8192;


    //map_user_address((uint64_t)stack,(uint64_t)stack-USERBASE,4096,(struct pml4t *)((uint64_t)p1->page_table+KERNBASE),0x07);
    //memset((void *)stack,0,4096);
    vma = bump(sizeof(struct vm_area_struct));
    vma->vma_type = STACK;
    vma->vma_file = NULL;
    vma->vma_start = (uint64_t)stack;
    vma->vma_end = (uint64_t)stack + 4095;
    //kprintf("\n Value of stack start: %x", vma->vma_start);
    //kprintf("\n Value of stack end: %x", vma->vma_end);
    tempvma = p1->mm->list_of_vmas;
    while (tempvma->next != NULL)
        tempvma = tempvma->next;
    //tempvma = vma;
    vma->prev = tempvma;
    tempvma->next = vma;
    p1->mm->num_of_vmas++;
    vma->next = NULL;
    //p1->rsp = (uint64_t)stack+4096;
    p1->ursp = (uint64_t)stack+4087;
    *((uint64_t *)p1->ursp)=(uint64_t)on_completion_pointer;
}


