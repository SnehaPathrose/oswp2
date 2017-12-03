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
int get_terminal_size();
void initialize_keyboard_buffer();
char* get_terminal_buf();
void reset_terminal();
#endif //COURSEPROJ_IO_H
