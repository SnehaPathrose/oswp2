#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define BIN "bin/"


//function to run background/foreground process
void bg_fg_process(char **command, char *envp[])
{
    int status,commandc;
    //char *env[]={"/rootfs/bin/",NULL};
    char *filename = 0;
    char concatstr[20];
    pid_t pid;
    int a;
    commandc = tokencount(command);
    //commandc=1;
    //filename = concat(BIN,command[0]);
    //if(access(filename,F_OK)< 0)
    //filename=concat("/usr/",filename);

    pid = fork();
    if(pid == 0)
    {
        /*char *path_values;
        path_values = getenv("PATH");
        char **tokenized_path = tokenize_path(path_values);
        for (int i = 0; tokenized_path[i] != 0x0; i++) {
            filename = strcat(tokenized_path[i], command[0], concatstr);
            a = access(filename, F_OK);
            if (a != -1)
                break;
        }*/
        filename = strcat(BIN,command[0],concatstr);
        a=access(filename,F_OK);
        if(a==-1)
        {
            puts("Command not found");
            puts(filename);
            exit(0);

        }
        else {
            execvpe(filename,(char * const *)command,(char * const *)envp);
        }



    }

    else if(pid > 0)
    {
        if(strcmp("&", command[commandc-1]) != 0)
        {
            waitpid(pid,&status);
        }
        else
        {
            return;
        }
    }
    //free(filename);
}



//function to map the command in the input string to its execution
void mapFunction(char *inputString, char *envp[]) {
    char **tokens;
    //char token[5][25];
    //char *t[5]={token[0],token[1],token[2],token[3],token[4]};
    // int status;
    tokens = tokenize(inputString);
    if (tokens[0][0] == '/' || tokens[0][0] == '.') {
        char *temp = (char *) malloc((strlen(tokens[0]) + 1) * sizeof(char));
        strcpy(temp, tokens[0]);
        strcpy(tokens[0], "sh");
        tokens[1] = (char *) malloc((strlen(temp) + 1) * sizeof(char));
        strcpy(tokens[1], temp);
    }

    /*//checking if it is change directory command
    if(compare("export",tokens[0])==0)
        setEnvironmentVariable(tokens[1]);
    else

        //checking if command contains pipe
    if(contain(inputString,'|')==1)
        pipecmd(inputString);
    else*/

    //other commands
    bg_fg_process(tokens, envp);

}

int main(int argc, char *argv[], char *envp[]) {
    char *sbush_string = getenv("PS1");
    write(1, sbush_string, 7);
    //write(1,"sbush> ",7);
    char inp[50], *stringInput;
    stringInput = inp;
    stringInput = gets(stringInput);
    while (strcmp(stringInput, "exit\n") != 0) {
        trim(stringInput);
        mapFunction(stringInput, envp);
        umemset(stringInput, 0, strlen(stringInput));
        sbush_string = getenv("PS1");
        write(1, sbush_string, strlen(sbush_string));
        stringInput = gets(stringInput);
    }
    write(1, "After everything", 16);
    return 1;

}