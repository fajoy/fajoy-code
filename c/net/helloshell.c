#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Array.h"
#include "String.h"
#include "Array.c"

typedef struct Cmd{
   int p_id;
   int argc;
   char **argv;
   int pipe[2];
   struct Cmd *parent;
   struct Cmd *prev;
   struct Cmd *next;
}Cmd;

Cmd getCmd(char *cmd,char *del ){
  Cmd c;
  Array *arr=split(cmd,del);
  c.argc =ToArray(arr,&c.argv);
  c.parent=NULL;
  c.prev=NULL;
  c.next=NULL;
  return c;
}


int spawn(Cmd *cmd)
{
   char *prog=cmd->argv[0];
   char **arg_list=cmd->argv;
   pid_t child;

   child = fork();
   if (child != 0) {
         cmd->p_id=child;
         //wait();
         return child;
      } else {
         execvp(prog, arg_list);
         fprintf(stderr, "spawn errorn");
         return -1;
     }
}

void showCmd(Cmd *cmd){
   int i;
   for(i=0;i<cmd->argc;i++){
      printf("argv[%d]=%s\n",i,cmd->argv[i]);
   }

}


int openStdOut(){
   //close(1);
   int fd=open("/dev/pts/0",1);
   return fd;
}

int openStdIn(){
   //close(0);
   int fd=open("/dev/pts/1",0);
   return fd;
}



int main(int argc,char *argv[],char *envp[]){
   char cmd[255];
   Cmd c;
   int i,j=0;
   do{
      printf("> ");
      fgets(cmd,255,stdin);
      i=strlen(cmd);
   }while(i==1);
   cmd[i-1]='\0';
   c=getCmd(cmd,"|");
   c.pipe[0]=0;
   c.pipe[1]=1;
   c.p_id=getpid();


   Cmd *subCmd=malloc(sizeof(Cmd)*c.argc);
   Cmd *prevCmd=NULL;
   Cmd *nextCmd=subCmd;
   i;
   for(i=0;i<c.argc;i++){
      printf("\n----sub arg[%d]=%s\n",i,c.argv[i]);
      *nextCmd=getCmd(c.argv[i]," ");

      if(prevCmd){
         prevCmd->next=nextCmd;
         nextCmd->prev=prevCmd;
      }
      nextCmd->parent=&c;
      prevCmd=nextCmd;
      showCmd(nextCmd);
      nextCmd++;
   }
   
   nextCmd=subCmd;  
   while(nextCmd){
   spawn(nextCmd);
   nextCmd=nextCmd->next;
   }

   wait();
   return 0;
}


