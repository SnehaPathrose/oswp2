//
// Created by sneha pathrose on 12/7/17.
//
#include <stdio.h>
#include <unistd.h>
char* fgets(int fileDescriptor, char* stringVal, int size) {
    char *temp = stringVal;
    char buf[1];
    int bytes;
    for( ; *temp != '\n'; ++temp) {

        buf[0] = '\0';
        bytes=read(fileDescriptor,buf, 1);
        if(bytes==EOF)
        {
            *temp=-1;
            *(temp+1)='\0';
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


