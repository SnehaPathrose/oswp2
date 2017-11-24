//
// Created by Toby Babu on 11/20/17.
//

#ifndef COURSEPROJ_KLIBC_H
#define COURSEPROJ_KLIBC_H

void memset(void* s, int num, int size);
int kstrlength(char *string);
int kstrcmp(char *string1, char *string2);
void kmemcpy(uint64_t *source, uint64_t *dest,uint64_t size);

#endif //COURSEPROJ_KLIBC_H