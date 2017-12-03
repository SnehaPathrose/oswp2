#ifndef _GDT_H
#define _GDT_H

#include <sys/contextswitch.h>

void init_gdt();
void set_tss_rsp(void *rsp);

#endif
