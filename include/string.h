//
// Created by Toby Babu on 12/3/17.
//

#ifndef COURSEPROJ_STRING_H
#define COURSEPROJ_STRING_H

#include <sys/defs.h>

int strlen(char *string);
int strcmp(char *string1, char *string2);
void memcpy(uint64_t *source, uint64_t *dest,uint64_t size);
char *strcpy(char *String1, char *String2);
char *strncpy(char *String1, char *String2, int n);
void umemset(void* s, int num, int size);

#endif //COURSEPROJ_STRING_H
