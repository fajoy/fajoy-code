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
   pid_t child=0;

   child = fork();
   if (child != 0) {
         cmd->p_id=child;
         return child;
      } else {

         if(cmd->prev!=NULL){
            dup2(cmd->prev->pipe[0],0);
         }else{
            dup2(cmd->parent->pipe[0],0);
         }
         if(cmd->next!=NULL){
            dup2(cmd->pipe[1],1);
         }else{
            dup2(cmd->parent->pipe[1],1);
         }
         dup2(1,2);
         Cmd *next=cmd->parent;
         while(next){
            close(next->pipe[0]);
            close(next->pipe[1]);
            next=next->next;
         }
         execvp(prog, arg_list);
         fprintf(stderr, "%s: command not found\n",prog);
         return -1;
     }
   return child;
}

void showCmd(Cmd *cmd){
   int i;
   for(i=0;i<cmd->argc;i++){
      printf("argv[%d]=%s\n",i,cmd->argv[i]);

   }

}




int main(int argc,char *argv[],char *envp[]){
   char cmd[255];
   Cmd c;
   c.pipe[0]=dup(0);
   c.pipe[1]=dup(1);
   c.p_id=getpid();
   while(1){
      int i,j=0;
      do{
         printf("%% ");
         memset(cmd,0,255);
         fgets(cmd,255,stdin);
         i=strlen(cmd);
         if(cmd[0]=='\0'&&i<=1)
            return 0;
      }while(i==1);
      cmd[i-1]='\0';
      fflush(stdout);
      Array *arr=split(cmd,"|");
      c.argc =ToArray(arr,&c.argv);
   
      Cmd *subCmd=malloc(sizeof(Cmd)*c.argc);
      c.next=subCmd;
      Cmd *prevCmd=NULL;
      Cmd *nextCmd=subCmd;
      for(i=0;i<c.argc;i++){
         *nextCmd=getCmd(c.argv[i]," ");
         if(prevCmd){
            prevCmd->next=nextCmd;
            nextCmd->prev=prevCmd;
         }
         nextCmd->parent=&c;
         prevCmd=nextCmd;
         pipe(nextCmd->pipe);
         nextCmd++;
      }
      fflush(stdout);
      nextCmd=subCmd;  
      int child_pid;
      while(nextCmd){
         child_pid=spawn(nextCmd);
         if(child_pid<0) //child exec error
            return -1;
         nextCmd=nextCmd->next;
      }
      //waitpid(final_pid, NULL, 0);

      nextCmd=subCmd;  
      while(nextCmd){
         close(nextCmd->pipe[0]);
         close(nextCmd->pipe[1]);
         if(!kill(nextCmd->p_id,0)){
            wait();
            nextCmd=subCmd;
         }
         nextCmd=nextCmd->next;
      }
      fflush(stdout);
      dup(c.pipe[0]);
      dup(c.pipe[1]);
   }
   return 0;
}


