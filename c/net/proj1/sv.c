#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#define PORTNUM 8000 

#include "Array.h"
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
   int stdo_fd=dup(1);
   int stdi_fd=dup(0);
   struct sockaddr_in dest; 
   struct sockaddr_in serv;
   int mysocket;         
   int socksize = sizeof(struct sockaddr_in);

   memset(&serv, 0, sizeof(serv));   
   serv.sin_family = AF_INET;        
   serv.sin_addr.s_addr = INADDR_ANY;
   serv.sin_port = htons(PORTNUM);       

   mysocket = socket(AF_INET, SOCK_STREAM, 0);

   bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));

   listen(mysocket, 1);
   int node_id=0;
   while(1){

         int consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
         close(0);
         close(1);
         dup(consocket);
         dup(consocket);
         node_id=fork();
         if(node_id!=0){//root
            close(0);
            close(1);
            close(consocket);
         }
         else{
            char *remote_ip=inet_ntoa(dest.sin_addr);
            char msg[255];
            char buffer[120];
            while(consocket)
            {
               dprintf(stdo_fd,"%s open\n",remote_ip);
               printf("hello!\n");
               fflush(stdout);
               Cmd c;
               c.pipe[0]=dup(consocket);
               c.pipe[1]=dup(consocket);
               c.p_id=getpid();
               while(1){
                  int i,j=0;
                  char buffer[255];
                  char *req;
                  printf("%% ");
                  fflush(stdout);
                  memset(buffer,0,255);
                  if(!fgets(buffer,255,stdin)){
                     dprintf(stdo_fd,"%s close\n",remote_ip);
                     break;
                  }
                  req=strtok(buffer,"\r\n");
                  if(!req)
                     continue;
                  dprintf(stdo_fd,"%%%s\n",req);
                  Array *arr=split(req,"|");
                  c.argc =ToArray(arr,&c.argv);
                  dprintf(stdo_fd,"(%d)argc\n",c.argc);
                  fflush(stdout);
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
                  close(0);
                  close(1);
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
               close(c.pipe[0]);
               close(c.pipe[1]);
               break;
            }
            close(consocket);
            close(0);
            close(1);
            close(2);
            break;
         }
   }

   close(mysocket);

   return 0;
}


