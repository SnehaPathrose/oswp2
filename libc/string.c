//
// Created by Toby Babu on 12/3/17.
//

#include <sys/defs.h>
#include <stdlib.h>

/*char *strcpy(char *String1, char *String2)
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

}*/

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

/*void memcpy(uint64_t *source, uint64_t *dest,uint64_t size)
{
    uint64_t i;
    size=size/8;
    for(i=0;i<=size;i++) {
        *(dest+i) = *(source+i);

    }
}*/

void umemset(void* s, int num, int size)
{
    unsigned char *uc =s;
    for (int i = 0; i < size; i++) {
        *uc = (unsigned char)num;
        uc++;
    }
}

//function to concatenate two strings

char *strcat(char *string1,char *string2) {
    int string1len = 0, string2len = 0, string3len = 0, i, j;
    char *concatstr;
    while(string1[string1len] != '\0')
        ++string1len;
    while(string2[string2len] != '\0')
        ++string2len;
    string3len = string1len + string2len+1;
    concatstr = (char *)malloc(string3len * (uint64_t)sizeof(char));
    umemset(concatstr, '\0', string3len * sizeof(char));
    for(i=0; i<string1len; i++)
        concatstr[i] = string1[i];
    for(j=0; j<string2len; j++)
        concatstr[i + j] = string2[j];
    concatstr[i + j]='\0';
    return concatstr;
}

void trim(char *string1) {
    int length = strlen(string1);
    string1[length - 1] = '\0';
}

//function to split the input string to tokens
char  **tokenize(char *string)
{
    char **tokens=(char **)malloc(5*sizeof(char *));
    int i = 0, j =0;
    tokens[j]=(char *)malloc(25*sizeof(char));
    for(int k = 0; k < 25; k++) {
        tokens[j][k] = '\0';
    }
    while(*string != '\0')
    {
        if ((*string != ' ') && (*string!='\n') )
        {
            *(*(tokens+j)+i) = *string;
            i++;
        }
        else
        {
            j++;
            if(*(string+1)!='\0') {
                tokens[j] = (char *) malloc(25 * sizeof(char));
                for(int k = 0; k < 25; k++) {
                    tokens[j][k] = '\0';
                }
            }
            i = 0;
        }
        string++;
    }
    //tokens[j][i] = '\0';
    return tokens;
}

/*void memcpychar(void *source, void *dest,uint64_t size)
{
    unsigned char *us =source;
    unsigned char *ud =dest;
    for (int i = 0; i < size; i++) {
        *ud = *us;
        us++;
        ud++;
    }
}*/

//function to find no of tokens
int tokencount(char **String)
{
    int i;
    for(i=0;String[i]!=NULL;i++);
    return i;
}

/*//function to check if a character is present in a string
int contain(char *String,char c)
{
    int i,found=0;
    for(i=0;String[i]!='\0';i++)
        if(String[i]==c)
        {
            found=1;
            break;
        }
    return found;
}

//function to split envValue by =
char** tokenizepath(char* envValue) {
    char **tokens=(char **)malloc(100*sizeof(char *));
    int i = 0, j =0;
    tokens[j]=(char *)malloc(1000*sizeof(char));
    //while(*(envValue++) != '=');

    while(*envValue != '\0')
    {
        if ((*envValue != ':'))
        {
            *(*(tokens+j)+i) = *envValue;
            i++;
        }
        else
        {
            j++;
            if(*(envValue+1)!='\0')
                tokens[j]=(char *)malloc(1000*sizeof(char));
            i = 0;
        }
        envValue++;
    }
    return tokens;

    //puts(envValue);
}*/

int atoi(char *s)
{
    int num=0;
    for(int i=0;i<strlen(s);i++)
    {
        num =num*10+(s[i]-'0');
    }
    return num;
}

