//
// Created by sneha pathrose on 12/7/17.
//
#include <stdio.h>
#include <unistd.h>
#include <string.h>

char* fgets(int fileDescriptor, char* stringVal, int size) {
    char *temp = stringVal;
    char buf[1];
    int bytes;
    for( ; *temp != '\n'; ++temp) {

        buf[0] = '\0';
        bytes=read(fileDescriptor,buf, 1);
        if(bytes==EOF)
        {
            if(stringVal[0]==0)
            {
                *temp = -1;
                *(temp + 1) = '\0';
            }
            else
                *(temp + 1) = '\0';
            return stringVal;
        }
        *temp = buf[0];
        if (*temp == '\n'){
            *(temp)='\0';
            return stringVal;
        }
    }
    return "";
}

#include <stdlib.h>

//static char *outputstring = (char *) (KERNBASE + 0xb8000);

/*
 * Function:  getargument
 * --------------------
 * Retrieves which of the first 6 arguments have to be used
 */
unsigned long
getargument(unsigned long a, unsigned long b, unsigned long c, unsigned long d, unsigned long e, int paramnum) {
    unsigned long f = 0;
    switch (paramnum) {
        case 2:
            return a;
        case 3:
            return b;
        case 4:
            return c;
        case 5:
            return d;
        case 6:
            return e;
    }
    return f;
}

/*
 * Function:  printf
 * --------------------
 * Print a string with variables on the screen
 * Supports variables of string, character, number, pointer and %x
 */
int printf(const char *fmt, ...) {
    char *value;
    int intvalue, length = 0, length2 = 0;
    unsigned long pointervalue;
    int rem[20], i;
    unsigned long arg1;
    unsigned long arg2;
    unsigned long arg3;
    unsigned long arg4;
    unsigned long arg5;
    unsigned long *arg6;
    __asm__("\t mov %%rsi,%0\n" : "=m"(arg1));
    __asm__("\t mov %%rdx,%0\n" : "=m"(arg2));
    __asm__("\t mov %%rcx,%0\n" : "=m"(arg3));
    __asm__("\t mov %%r8,%0\n" : "=m"(arg4));
    __asm__("\t mov %%r9,%0\n" : "=m"(arg5));
    __asm__("\t lea 208(%rsp),%r11\n");
    char *buffer = (char *)malloc(256);
    int noofarg = 1;
    for (; fmt[length] != 0x0; length2 += 1, length += 1) {
        if (fmt[length] == '%') {
            noofarg++;
            //write logic to get the noofarg argument from stack
            switch (fmt[length + 1]) {
                case 's': // handling %s variable in main string
                    if (noofarg > 6) {
                        __asm__("\t mov %%r11, %0\n": "=m"(arg6));
                        __asm__("\t add $8,%r11\n");
                        value = (char *) *arg6;
                    } else {
                        value = (char *) getargument(arg1, arg2, arg3, arg4, arg5, noofarg);
                    }
                    for (int j = 0; (value)[j] != '\0'; length2 ++, j++)
                        buffer[length2] = (value)[j];
                    //length += 1;
                    break;
                case 'c': // handling %c in main string
                    if (noofarg > 6) {
                        __asm__("\t mov %%r11, %0\n": "=m"(arg6));
                        __asm__("\t add $8,%r11\n");
                        buffer[length2] = (char) *arg6;
                    } else {
                        buffer[length2] = (char) getargument(arg1, arg2, arg3, arg4, arg5, noofarg);
                    }
                    //length +=2;
                    break;
                case 'd': // handling %d in main string
                    if (noofarg > 6) {
                        __asm__("\t mov %%r11, %0\n": "=m"(arg6));
                        __asm__("\t add $8,%r11\n");
                        intvalue = (int) *arg6;
                    } else {
                        intvalue = (int) getargument(arg1, arg2, arg3, arg4, arg5, noofarg);
                    }
                    i = 0;
                    if (intvalue == 0) {
                        buffer[length2] = '0';
                    } else {
                        while (intvalue != 0) {
                            rem[i++] = intvalue % 10 + '0';
                            intvalue = intvalue / 10;
                        }
                        for (int j = i - 1; j >= 0; j--, length2 += 1)
                            buffer[length2] = rem[j];
                    }
                    break;
                case 'p': // handling %p in main string
                    buffer[length2] = '0';
                    length2 += 1;
                    buffer[length2] = 'x';
                    length2 += 1;
                case 'x': // handling %x in main string
                    i = 0;

                    if (noofarg > 6) {
                        __asm__("\t mov %%r11, %0\n": "=m"(arg6));
                        __asm__("\t add $8,%r11\n");
                        pointervalue = *arg6;
                    } else {
                        pointervalue = getargument(arg1, arg2, arg3, arg4, arg5, noofarg);
                    }
                    if (pointervalue == 0) {
                        buffer[length2] = '0';
                    } else {
                        while (pointervalue != 0) {
                            rem[i++] =
                                    (pointervalue % 16) < 10 ? (pointervalue % 16) + '0' : ((pointervalue % 16) % 10) +
                                                                                           'a';
                            pointervalue = pointervalue / 16;
                        }
                        for (int j = i - 1; j >= 0; j--, length2 += 1)
                            buffer[length2] = rem[j];
                    }
                    break;
                }
            fmt++;
        } else {
            buffer[length2] = fmt[length];
        }
    }
    //*buffer = '\0';
    if (length > 0) {
        buffer[length2] = '\0';
        write(1, buffer, length2);
        return length;
    }
    return -1;

}











