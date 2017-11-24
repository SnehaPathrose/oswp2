#ifndef _GDT_H
#define _GDT_H

#include <sys/contextswitch.h>

void init_gdt();
void set_tss_rsp(void *rsp);
//void switch_to_ring_3(struct PCB *tss);

#endif
