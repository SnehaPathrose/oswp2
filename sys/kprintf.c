#include <sys/kprintf.h>

static char* outputstring = (char*)0xb8000;
/*
 * Function:  getargument 
 * --------------------
 * Retrieves which of the first 6 arguments have to be used
 */
unsigned long getargument(unsigned long a, unsigned long b, unsigned long c, unsigned long d, unsigned long e, int paramnum) {
    unsigned long f=0;
    switch(paramnum) {
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
 * Function:  kprintf 
 * --------------------
 * Print a string with variables on the screen
 * Supports variables of string, character, number, pointer and %x
 */
void kprintf(const char *fmt, ...)
{
    char* value;
    int intvalue, length = 2;
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
    int noofarg=1;
    for (;*fmt!='\0';outputstring += 2,fmt++, length += 2)
    {
        if(*fmt=='%')
        {
            noofarg++;
            //write logic to get the noofarg argument from stack
            switch(*(fmt+1))
            {
                case 's': // handling %s variable in main string
                    if(noofarg > 6) {
                        __asm__("\t mov %%r11, %0\n": "=m"(arg6));
                        __asm__("\t add $8,%r11\n");
                        value = (char*)*arg6;
                    }
                    else {
                        value = (char*)getargument(arg1, arg2, arg3, arg4, arg5, noofarg);
                    }
                    for(int j=0;((char*)value)[j]!='\0';outputstring += 2,j++,length+=2)
                        *outputstring=((char*)value)[j];
                    outputstring -= 2;
                    length -= 2;
                    break;
                case 'c': // handling %c in main string
                    if(noofarg > 6) {
                        __asm__("\t mov %%r11, %0\n": "=m"(arg6));
                        __asm__("\t add $8,%r11\n");
                        *outputstring = (unsigned long)*arg6;
                    }
                    else {
                        *outputstring = (unsigned long)getargument(arg1, arg2, arg3, arg4, arg5, noofarg);
                    }

                    break;
                case 'd': // handling %d in main string
                    if(noofarg > 6) {
                        __asm__("\t mov %%r11, %0\n": "=m"(arg6));
                        __asm__("\t add $8,%r11\n");
                        intvalue = (int)*arg6;
                    }
                    else {
                        intvalue = (int)getargument(arg1, arg2, arg3, arg4, arg5, noofarg);
                    }
                    i = 0;

                    while(intvalue!=0)
                    {
                        rem[i++] = intvalue % 10 + '0';
                        intvalue = intvalue / 10;
                    }
                    for(int j = i - 1;j >= 0;outputstring += 2, j--, length += 2)
                        *outputstring = rem[j];
                    outputstring -= 2;
                    length -= 2;
                    break;
                case 'p': // handling %p in main string
                    *outputstring = '0';
                    outputstring += 2;
                    *outputstring = 'x';
                    outputstring += 2;
                    length += 4;
                case 'x': // handling %x in main string
                    i = 0;

                    if(noofarg > 6) {
                        __asm__("\t mov %%r11, %0\n": "=m"(arg6));
                        __asm__("\t add $8,%r11\n");
                        pointervalue = (unsigned long)*arg6;
                    }
                    else {
                        pointervalue = (unsigned long)getargument(arg1, arg2, arg3, arg4, arg5, noofarg);
                    }
                    while(pointervalue!=0)
                    {
                        rem[i++] = (pointervalue%16) < 10 ? (pointervalue%16) + '0' : ((pointervalue % 16) % 10) + 'a';
                        pointervalue = pointervalue / 16;
                    }
                    for(int j = i - 1;j >= 0;outputstring += 2, j--, length += 2)
                        *outputstring = rem[j];
                    outputstring -= 2;
                    length -= 2;
                    break;
            }
            fmt++;
        }
        else if (*fmt == '\n')
        {
            outputstring -= length;
            outputstring += 160;

        }
        else if (*fmt=='\r')
        {
            outputstring -= length;
        }
        else
        {
            *outputstring = *fmt;
        }
    }
    *outputstring = '\0';
}


