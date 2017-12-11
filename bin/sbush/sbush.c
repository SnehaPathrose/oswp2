#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define BIN "bin/"


//function to run background/foreground process
void bg_fg_process(char **command)
{
    int status,commandc;
    char *env[]={"/rootfs/bin/",NULL};
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
        /* if(strcmp("&", command[commandc-1]) == 0)
             command[commandc-1]=NULL;*/
        /*if((strcmp(command[0],"ls")==0) ||
           (strcmp(command[0],"cat")==0) ||
           (strcmp(command[0],"kill")==0) ||
           (strcmp(command[0],"ps")==0) ||
           (strcmp(command[0],"echo")==0) ||
           (strcmp(command[0],"sleep")==0) ||
           (strcmp(command[0],"pwd")==0) ||
           (strcmp(command[0],"cd")==0))
        {*/
        filename = strcat(BIN,command[0],concatstr);
        a=access(filename,F_OK);
        if(a==-1)
        {
            puts("Command not found");
            puts(filename);
            exit(0);

        }
        else {
            execvpe(filename,(char * const *)command,(char * const *)env);
        }

        /*if(access(filename, F_OK) < 0)
        {
            int i = 0;
            char** abc = tokenizepath(getenv("PATH"));
            while(abc[i] != '\0') {
                abc[i] = concat(abc[i], "/");
                abc[i] = concat(abc[i], command[0]);
                if ((strcmp(abc[i], "/bin/ls") == 0) || (strcmp(abc[i], "/bin/cat") == 0))
                {
                    i++;
                    continue;
                }
                if(access(abc[i], F_OK) >= 0)
                    break;
                *//*filename = fopen(abc[i], O_RDONLY);
                    if (filename > 0)
                        break;*//*

                    i++;
                }
                *//*if (filename < 0) {
                    puts("File Not Found\n");
                    return 1;
                }*//*
                filename = abc[i];
            }*/

    }
        /*else {
            puts("Command not found");
            exit(0);
        }*/
        //char *envp[] = { NULL };
        //execvpe(filename,(char * const *)command/*, envp*/);
        //}
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
void mapFunction(char* inputString)
{
    char **tokens;
    //char token[5][25];
    //char *t[5]={token[0],token[1],token[2],token[3],token[4]};
    // int status;
    tokens = tokenize(inputString);
    if(tokens[0][0]=='/' || tokens[0][0]=='.') {
        char *temp=(char *)malloc((strlen(tokens[0]) + 1) * sizeof(char));
        strcpy(temp,tokens[0]);
        strcpy(tokens[0],"sh");
        tokens[1]=(char *)malloc((strlen(temp) + 1) * sizeof(char));
        strcpy(tokens[1],temp);
    }

    /*//checking if it is change directory command
    if(compare("cd", tokens[0]) == 0)
    {
        cd(tokens[1]);
    }
    else
        //checking if it is executing a script
    if(compn(tokens[0],"./",2)==0)
    {
        for(i=0;i<strlength(tokens[0]);i++)
            tokens[0][i]=tokens[0][2+i];
        tokens[0][i]='\0';
        status=execscripts(tokens);
        if(status==1)
            puts("Script could not be executed\n");

    }
    else
        //checking if PATH/PS1 variables are being set
    if(compare("export",tokens[0])==0)
        setEnvironmentVariable(tokens[1]);
    else

        //checking if command contains pipe
    if(contain(inputString,'|')==1)
        pipecmd(inputString);
    else*/

    //other commands
    bg_fg_process(tokens);

}

int main(int argc, char *argv[], char *envp[]) {
    write(1,"sbush> ",7);
    char inp[50], *stringInput;
    stringInput = inp;
    stringInput = gets(stringInput);
    while(strcmp(stringInput, "exit\n")!= 0)
    {
        trim(stringInput);
        mapFunction(stringInput);
        umemset(stringInput, 0, strlen(stringInput));
        write(1,"sbush> ",7);
        stringInput = gets(stringInput);
    }
    write(1,"After everything",16);

    //w=read(1,test_string,10);
    /*if(w==1)
    {
    w=0;
    }*/


    /*int f,w,e;
    f=fork();

    if(f==0) {
        w = write(1, "\nsbush> from child", 25);
    }
    else{
        // w = write(1, "\nsbush> from parent", 25);
        char *param[]={"bin/ls","hey","123",0};
        //char *env[]={"envhey","env123",0};
        e=execvpe(param[0],param);
        if(e==1)
            write(1,"exec failed\n",50);
    }

    if(w==1)
    {
        w=0;
    }

    return 0;*/
    return 1;

}