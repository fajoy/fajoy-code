#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Array.h"
#include "String.h"
#include "Array.c"

typedef struct{
   int p_id;
   int argc;
   char **argv;
   int pipe[2];
   struct  Cmd *parnet;
   struct Cmd *prev;
   struct Cmd *next;
}Cmd;

Cmd getCmd(char *cmd,char *del ){
  Cmd c;
  Array *arr=split(cmd,"|");
  c.argc =ToArray(arr,&c.argv);
  return c;
}


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
   Cmd c;
   int i=0;
   do{
      printf("> ");
      fgets(cmd,255,stdin);
   }while(strlen(cmd)==1);
   c=getCmd(cmd,"|");
   Cmd *subCmd=malloc(sizeof(Cmd)*c.argc);
   i;
   for(i=0;i<c.argc;i++){
      printf("arg[%d]=%s\n",i,c.argv[i]);

   }
   return 0;
}


