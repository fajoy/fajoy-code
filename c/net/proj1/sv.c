#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>   
#include  <fcntl.h>
#define PORTNUM 8000 
#define queMAX 32767
#include <string.h>
#include "Array.h"
#include "Array.c"
char PATH[255]="PATH=bin:.";
char PWD[255]="";
char binPATH[255];
char *env[]={PATH,PWD,""};
int stdo_fd;
int stdi_fd;
typedef struct Cmd{
   int p_id;
   int argc;
   char **argv;
   int pipe[2];
   int dstpipe[2];
   int qi;
   int dst;
   struct Cmd *parent;
   struct Cmd *first;
   struct Cmd *prev;
   struct Cmd *next;
}Cmd;

void freeCmd(Cmd *cmd){
   free(cmd);
}

Cmd getCmd(char *cmd,char *del ){
  Cmd c;
  Array *arr=split(cmd,del);
  c.argc =ToArray(arr,&c.argv);
  c.pipe[0]=-1;
  c.pipe[1]=-1;
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

   dup2Cmd(cmd);
   child = fork();

   if (child != 0) {
      if(cmd->prev!=NULL){
      close(cmd->prev->pipe[0]);
      close(cmd->prev->pipe[1]);
      }

      cmd->p_id=child;

      return child;
   } else {
      close(0);
      close(1);
      dup(cmd->dstpipe[0]);
      dup(cmd->dstpipe[1]);
      Cmd *next=cmd->first;
      while(next){
         close(next->pipe[0]);
         close(next->pipe[1]);
         next=next->next;
      }

         execve(strcat(binPATH,prog), arg_list,env);
        //fprintf(stderr, "%s: command not found\n",prog);
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

int pipeCmd(Cmd *cmd){
   pipe(cmd->pipe);
   int i,fd=1;
   for (i=1;i+1<cmd->argc;i++){
      if(strncmp(cmd->argv[i],">",1)==0)
      {
         char * fpath=cmd->argv[i+1];
         close(cmd->pipe[1]);
         fd=open(fpath,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
         cmd->argv[i]=NULL;
         cmd->dst=-1;
      }
   }
}
void checkPipe(Cmd *cmd){
   if(cmd->pipe[0]==-1)
      pipeCmd(cmd);
}
int ddup2(int src ,int dst){
   return dup2(src,dst);
}
int dup2Cmd(Cmd *cmd){
   checkPipe(cmd);
   int li =cmd->pipe[0];
   int lo =cmd->pipe[1];
   Cmd *prev,*next;

     if(cmd->prev==NULL)
        prev=cmd->parent;
     else 
        prev=cmd->prev;


     if(cmd->dst==-1)
        next=cmd;
     else if(cmd->next==NULL)
        next=cmd->parent;
     else if(cmd->dst==cmd->qi)
        next=cmd;
     else if(cmd->dst==cmd->next->qi)
        next=cmd;
     else{
        Cmd *np=cmd->next;
        while (np){
           if(cmd->dst==np->qi)
              break;
           np=np->next;
        }
        next= np->prev;

     }

   /*
     if(cmd->prev==NULL)
         ddup2(cmd->parent->pipe[0],i);
     else 
         ddup2(cmd->prev->pipe[0],i);

      if(cmd->dst==-1){
         close(1);
         dup(cmd->pipe[1]);
      }
      else if(cmd->next==NULL)
         ddup2(cmd->parent->pipe[1],o);
      else if(cmd->dst==cmd->qi)
         ddup2(cmd->pipe[1],o);
      else if(cmd->dst==cmd->next->qi)
         ddup2(cmd->pipe[1],o);
      else{
         Cmd *np=cmd->next;
         while (np){
            if(cmd->dst==np->qi)
               break;
            np=np->next;
         }
         ddup2(np->prev->pipe[1],o);
         }
     */ 

   checkPipe(prev);
   checkPipe(next);
   int ri=prev->pipe[0];
   int ro=next->pipe[1];
   cmd->dstpipe[0]=ri;
   cmd->dstpipe[1]=ro;
   dprintf(stdo_fd,"i%d,o%d ",cmd->pipe[0],cmd->pipe[1]);
   dprintf(stdo_fd,"%d-%d %d-%d\n",ri,0,lo,1);
     
}
int main(int argc,char *argv[],char *envp[]){
   chdir("ras");
   sprintf(PWD,"PWD=%s",getenv("PWD"));
   sprintf(binPATH,"%s/ras/bin/",getenv("PWD"));
   printf("%s\n",PWD) ;
   printf("%s\n",PATH) ;
   printf("%s\n",binPATH) ;
   stdo_fd=dup(1);
   stdi_fd=dup(0);
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

   listen(mysocket, 5);
   int node_id;
   int flowNo=0;
   while(1){
      dprintf(stdo_fd,"%d+ start accet \n",flowNo);
      int consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
      if(consocket>=0){
         dprintf(stdo_fd,"%d+ ok\n",flowNo++,consocket);
         close(0);
         close(1);
         dup(consocket);
         dup(consocket);
         node_id=0;
         node_id=fork();
         if(node_id>0){//root
            close(0);
            close(1);
            close(consocket);
            dup(stdo_fd);
            dup(stdo_fd);
            continue;
         }else{
            close(mysocket);
            close(stdi_fd);
            char *remote_ip=inet_ntoa(dest.sin_addr);
            char msg[255];
            char buffer[255];

            Cmd *cmd_que[queMAX];
            memset(cmd_que,0,queMAX);
            int qi=0;

            dprintf(stdo_fd,"(%d)%s open\n",getpid(),remote_ip);
            fflush(stdout);

            Cmd *c=(Cmd*)malloc(sizeof(Cmd));
            c->pipe[0]=dup(consocket);
            c->pipe[1]=dup(consocket);
            c->p_id=getpid();
            while(1){
               int i,j=0;
               char buffer[255];
               char *req;
               printf("%% ");
               fflush(stdout);
               memset(buffer,0,255);
               if(!fgets(buffer,255,stdin)){
                  break;
               }
               req=strtok(buffer,"\r\n/");
               cmd_que[qi++]=NULL;
               qi%=queMAX;
               dprintf(stdo_fd,"(%d+%d)%%%s\n",getpid(),qi,req);
               if(!req)
                  continue;
               if(strcmp(req,"printenv PATH")==0)
               {
                  printf("%s\n",PATH);
                  continue;
               }else if(strncmp(req,"setenv PATH",11)==0){
                  sscanf(req,"setenv PATH %s",&PATH[5]);
                  continue;
               }else if(strncmp(req,"exit",4)==0){
                  break;
               }
               int delay=0;
               Array *arr=split(req,"|!");
               c->argc =ToArray(arr,&c->argv);
               c->qi=qi;
               fflush(stdout);
               Cmd *prevCmd=cmd_que[qi];
               if(prevCmd!=NULL){
                  c->first=prevCmd->first;
                  c->next=prevCmd->first;
               }else{
                  c->first=NULL;
                  c->first=NULL;
               }
               Cmd *subCmd=malloc(sizeof(Cmd)*c->argc);
               Cmd *nextCmd=subCmd;
               for(i=0;i<c->argc;i++){
                  *nextCmd=getCmd(c->argv[i]," ");
                  nextCmd->qi=qi;
                  nextCmd->dst=qi;
                  if(c->first==NULL){
                     c->first=nextCmd;
                  }
                  if (i==0)
                  {
                     c->next=nextCmd;
                  }

                  if(i+1==c->argc&&i!=0){//final
                     delay=atoi(nextCmd->argv[0]);
                     if(delay>0)
                        break;
                   }

                  if(prevCmd){
                     prevCmd->next=nextCmd;
                     nextCmd->prev=prevCmd;
                  }
                  nextCmd->parent=c;
                  nextCmd->first=c->first;
                  prevCmd=nextCmd;
                  nextCmd++;
               }
               if(delay>0){
                  int dstqi=(qi+delay)%queMAX;
                  nextCmd=prevCmd;
                  prevCmd=cmd_que[dstqi];
                  nextCmd->dst=dstqi;
                  cmd_que[dstqi]=nextCmd;
                  if(prevCmd){
                     prevCmd->next=c->next;
                     c->next->prev=prevCmd;
                     nextCmd=prevCmd->next;
                     while(nextCmd){
                        nextCmd->first=prevCmd->first;
                        nextCmd=nextCmd->next;
                     }
                  }
                  dprintf(stdo_fd,"(%d->%d)+%d\n",getpid(),delay,dstqi );
                  continue;
               }



               nextCmd=c->first;  
               int child_pid;
               while(nextCmd){
                  dprintf(stdo_fd,"%s |",nextCmd->argv[0]);
                  child_pid=spawn(nextCmd);
                  if(child_pid<0) //child exec error
                     return -1;
                  nextCmd=nextCmd->next;
               }
               dprintf(stdo_fd," \n");

               nextCmd=c->first;  
               while(nextCmd){
                  close(nextCmd->pipe[0]);
                  close(nextCmd->pipe[1]);
                  if(!kill(nextCmd->p_id,0)){
                     wait();
                  }
                  nextCmd=nextCmd->next;
               }
              /* 
               nextCmd=c->first;  
               while(nextCmd){
                  prevCmd=nextCmd;
                  nextCmd=nextCmd->next;
                  freeCmd(prevCmd);
               }
               */

            }
            close(c->pipe[0]);
            close(c->pipe[1]);
            close(consocket);
            close(0);
            close(1);
            close(2);
            dprintf(stdo_fd,"(%d)%s close\n",getpid(),remote_ip);
            return 0;
         }

         break;
      }
   }
   close(mysocket);
   dprintf(stdo_fd,"server stop\n");
   return 0;
}


