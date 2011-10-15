#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Array.h"
#include "String.h"
#include "Array.c"
int spawn(char *prog, char **arg_list)
{
   pid_t child;

   child = fork();

   if (child != 0) {
         wait();
         return child;
      } else {
            execvp(prog, arg_list);
            fprintf(stderr, "spawn errorn");
            return -1;
         }
}

int main(int argc,char *argv[],char *envp[]){
   char cmd[255];
   printf("cmd:");
   scanf("%[^\n]",cmd);
   Array *arr=split(cmd," ");
   char **arg;
   int arg_size=ToArray(arr,&arg);
   int i;
   for(i=0;i<arg_size;i++)
      printf("arg[%d]=%s\n",i,arg[i]);
   int child_id= spawn(arg[0],arg);
   printf("(%d)child end",child_id);
   return 0;
}


