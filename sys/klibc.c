//
// Created by Toby Babu on 11/20/17.
//

#include <sys/allocator.h>
#include <sys/defs.h>
/*
 * Function:  memset
 * --------------------
 * Write size number of bytes with value num at the locatio starting
 * from s
 */
void memset(void* s, int num, int size)
{
    unsigned char *uc =s;
    for (int i = 0; i < size; i++) {
        *uc = (unsigned char)num;
        uc++;
    }
}

int kstrlength(char *string)
{
    int stringlen = 0;
    while(string[stringlen]!='\0')
    {
        ++stringlen;
    }
    return stringlen;
}

int kstrcmp(char *string1, char *string2)
{
    int i;
    if(kstrlength(string1)!= kstrlength(string2))
        return -1;
    for(i=0;string1[i]!='\0';i++)
        if(string1[i]!=string2[i])
            return -1;
    return 0;
}

void kmemcpy(uint64_t *source, uint64_t *dest,uint64_t size)
{
    uint64_t i;
    for(i=0;i<size;i++) {
        *(dest+i) = *(source+i);

    }
}

//function to concatenate two strings

char *concat(char *string1,char *string2) {
    int string1len = 0, string2len = 0, string3len = 0, i, j;
    char *concatstr;
    while(string1[string1len] != '\0')
        ++string1len;
    while(string2[string2len] != '\0')
        ++string2len;
    string3len = string1len + string2len+1;
    concatstr = (char *)bump(string3len * (uint64_t)sizeof(char));
    for(i=0; i<string1len; i++)
        concatstr[i] = string1[i];
    for(j=0; j<string2len; j++)
        concatstr[i + j] = string2[j];
    concatstr[i + j]='\0';
    return concatstr;
}