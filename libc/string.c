//
// Created by Toby Babu on 12/3/17.
//

#include <sys/defs.h>

char *strcpy(char *String1, char *String2)
{
    int i;
    for (i=0;String2[i]!='\0';i++)
        String1[i]=String2[i];
    String1[i]='\0';
    return String1;

}

char *strncpy(char *String1, char *String2, int n)
{
    int i;
    for (i = 0; String2[i]!='\0' && i < n; i++)
        String1[i]=String2[i];
    String1[i]='\0';
    return String1;

}

int strlen(char *string)
{
    int stringlen = 0;
    while(string[stringlen]!='\0')
    {
        ++stringlen;
    }
    return stringlen;
}

int strcmp(char *string1, char *string2)
{
    int i;
    if(strlen(string1)!= strlen(string2))
        return -1;
    for(i=0;string1[i]!='\0';i++)
        if(string1[i]!=string2[i])
            return -1;
    return 0;
}

void memcpy(uint64_t *source, uint64_t *dest,uint64_t size)
{
    uint64_t i;
    size=size/8;
    for(i=0;i<=size;i++) {
        *(dest+i) = *(source+i);

    }
}

void umemset(void* s, int num, int size)
{
    unsigned char *uc =s;
    for (int i = 0; i < size; i++) {
        *uc = (unsigned char)num;
        uc++;
    }
}

/*
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
}*/
