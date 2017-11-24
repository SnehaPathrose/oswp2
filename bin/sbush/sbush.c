#include <stdio.h>


int main(int argc, char *argv[], char *envp[]) {
  int w;
  w=write(1,"\nsbush> from Userland",25);
  if(w==1)
  {
    w=0;
  }
  return 1;

}