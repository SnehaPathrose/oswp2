//
// Created by Toby Babu on 11/19/17.
//

#ifndef COURSEPROJ_IO_H
#define COURSEPROJ_IO_H

#include "defs.h"
#include "virtualmem.h"

void kprintf(const char *fmt, ...);
char* get_buffer_value();
void set_buffer_value(char *output_value);
void kscanf(uint8_t keyscancode);
char* read_from_ip(int read_size, char* buf);
#endif //COURSEPROJ_IO_H
