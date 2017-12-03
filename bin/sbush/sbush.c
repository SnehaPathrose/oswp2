#include <stdio.h>
#include <unistd.h>
#include <string.h>

//char *cmdprompt="sbush>";
int main(int argc, char *argv[], char *envp[]) {
    //int w;
    //w=write(1,"\nsbush> from Userland",25);
    //w = fork();
    //char test_string[10];
    //getchar();
    //w=read(1,test_string,10);
    /*gets(test_string);
    puts(test_string);
    w = write(1,"sbush>",25);*/
    write(1,"sbush> ",7);
    char inp[50], *stringInput;
    stringInput = inp;
    stringInput = gets(stringInput);
    while(strcmp(stringInput, "exit\n")!= 0)
    {
        //mapFunction(stringInput);
        umemset(stringInput, 0, strlen(stringInput));
        write(1,"sbush> ",7);
        stringInput = gets(stringInput);
    }
    write(1,"After everything",7);
    //w=read(1,test_string,10);
    /*if(w==1)
    {
    w=0;
    }*/
    return 1;

}