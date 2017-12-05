#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define BIN "/bin/"


//function to run background/foreground process
void bg_fg_process(char **command)
{
    int ret,status,commandc;
    char *filename = 0;
    pid_t pid;
    commandc = tokencount(command);
    //filename = concat(BIN,command[0]);
    //if(access(filename,F_OK)< 0)
        //filename=concat("/usr/",filename);
    pid = fork();
    if(pid == 0)
    {
        if(strcmp("&", command[commandc-1]) == 0)
            command[commandc-1]=NULL;
        if((strcmp(command[0],"ls")==0) || (strcmp(command[0],"cat")==0))
        {
            filename = concat("bin/",command[0]);

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
        else {
            exit(0);
        }
        //char *envp[] = { NULL };
        ret=execvpe(filename,(char * const *)command/*, envp*/);
        if(ret<0)
            puts("Command not found\n");
        exit(0);
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
    else if(pid<0)
    {
        puts("Error in forking\n");
    }
    //free(filename);
}

//function to map the command in the input string to its execution
void mapFunction(char* inputString)
{
    char **tokens;
    //int status,i;
    tokens = tokenize(inputString);

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

int main(int argc, char *argv[]) {
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
    write(1,"After everything",7);

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