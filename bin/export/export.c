//
// Created by Toby Babu on 12/12/17.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char* argv[], char *envp[])
{
    //setenv(argv[1], argv[2]);

    /*char *envVariableName = (char *)malloc(25);
    char *envValue = (char *)malloc(75);
    char *joinedValue = (char *)malloc(75);
    char *existingVar = (char *)malloc(75);
    int i = 0, varFound = 0, existVarFound = 0, j = 0;
    while(*argv[1] != '\0')
    {
        if (*argv[1] != '=')
        {
            if (varFound == 0)
            {
                *(envVariableName + i) = *argv[1];
                i++;
            }
            else
            {
                if (*argv[1] == '$')
                {
                    argv[1]++;
                    existVarFound = 1;
                    while(*argv[1] != ':')
                    {
                        *(existingVar + j) = *argv[1];
                        j++;
                        argv[1]++;
                    }
                }
                *(envValue + i) = *argv[1];
                i++;
            }

        }
        else
        {
            varFound = 1;
            i = 0;
        }
        argv[1]++;
    }

    if (existVarFound == 1)
    {
        joinedValue = strcat(getenv(existingVar), envValue, joinedValue);
    }
    int abc = setenv(envVariableName, joinedValue);
    if(abc == -1)
        puts("Environment variable could not be found\n");*/
    return 0;
}