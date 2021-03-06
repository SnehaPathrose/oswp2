#ifndef _STDIO_H
#define _STDIO_H

static const int EOF = -1;

int putchar(int c);
int puts(const char *s);
int printf(const char *format, ...);
int getchar();
char *gets(char *s);
char* fgets(int fileDescriptor, char* stringVal, int size);

#endif

