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
#define queMAX 32767
#include <string.h>
#include "Array.h"
#define ndebug
int PORTNUM=8000;
char PATH[255]="PATH=bin:.";
char PWD[255]="";
char binPATH[255];
char *env[]={PATH,PWD,""};

int erro_fd;
int stdo_fd;
int stdi_fd;

Array *split(char *str, const char *del) {
   char *root=strdup(str);
   char *sub=strtok(root, del);
   Array *list=newArray(100);
   int i;
   while(sub != NULL) {
     i=list->length;
     ArrayPush(list,sub);
     sub = strtok(NULL, del);
     char*ss=ArrayGet(list,i,char*);
     //printf("list[%d]=%s\n",i,ss);
   }
   return list;
} 

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
  if (arr->length!=0)
     c.argc =ToArray(arr,&c.argv);
  else
     c.argc=0;
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
      if(child>0){
      if(cmd->prev!=NULL){

      if(cmd->pipe[1]==erro_fd){
         erro_fd=2;
      }
      else{
         close(cmd->prev->pipe[0]);
         close(cmd->prev->pipe[1]);
      }
      }

      cmd->p_id=child;
      }
      return child;
   } else {

#ifdef debug
   dprintf(stdo_fd,"(%d)%s  i%d,o%d ",getpid(),cmd->argv[0],cmd->pipe[0],cmd->pipe[1]);
   dprintf(stdo_fd,"%d-%d %d-%d\n",cmd->dstpipe[0],0,cmd->dstpipe[1],1);
#endif
      close(0);
      close(1);
      dup(cmd->dstpipe[0]);
      dup(cmd->dstpipe[1]);
      if(erro_fd==2){
         close(2);
         dup(cmd->dstpipe[1]);
      }
      else
      {
         //close(2);
         //dup2(erro_fd,2);
      }
      Cmd *next=cmd->first;
      while(next){
         close(next->pipe[0]);
         close(next->pipe[1]);
         next=next->next;
      }
         execve(strcat(binPATH,prog), arg_list,env);
         //fprintf(stderr, "%s: command not found\n",prog);
      if(erro_fd!=2)
         dprintf(erro_fd,"Unknown command: [%s].\n",prog);
      else
         fprintf(stdout,"Unknown command: [%s].\n",prog);
         return 0;
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
         if(strncmp(cmd->argv[i],">>",2)==0)
         fd=open(fpath,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
         else
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
int indexOf(char *str1,char *str2)  
{  
    char *p=str1;  
    int i=0;  
    p=strstr(str1,str2);  
    if(p==NULL)  
        return -1;  
    else{  
            while(str1!=p)  
            {  
                        str1++;  
                        i++;  
                    }  
        }  
    return i;  
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
}
int main(int argc,char *argv[],char *envp[]){

   printf("Input Server Port:");
   char input[10];
   fgets(input,9,stdin);
   if(atoi(input)!=0)
      PORTNUM=atoi(input);
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
         fflush(stdout);
         fflush(stdin);
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
            erro_fd=2;
            close(mysocket);
            close(stdi_fd);
            char *remote_ip=inet_ntoa(dest.sin_addr);
            int remote_port=dest.sin_port;
            char msg[255];
            char buffer[255];

            Cmd *cmd_que[queMAX];
            memset(cmd_que,0,queMAX);
            int qi=0;

            dprintf(stdo_fd,"(%d)%s open\n",getpid(),remote_ip);
            
            printf("****************************************\n");
            printf("** Welcome to the information server. **\n");
            printf("****************************************\n");
            printf("Your From %s:%d\n",remote_ip,remote_port);
            fflush(stdout);
            
            Cmd *c=(Cmd*)malloc(sizeof(Cmd));
            c->pipe[0]=dup(consocket);
            c->pipe[1]=dup(consocket);
            c->p_id=getpid();
            while(1){
               int i,j;
               char buffer[255];
               char *req;
               printf("%% ");
               memset(buffer,0,255);
               fflush(stdout);
               fflush(stdin);
               if(!fgets(buffer,255,stdin)){
                  break;
               }
               req=strtok(buffer,"\r\n/");
               cmd_que[qi++]=NULL;
               qi%=queMAX;

#ifdef debug
               dprintf(stdo_fd,"(%d+%d)%%%s\n",getpid(),qi,req);
#endif
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
               int has_err_pipe=indexOf(req,"!");

#ifdef debug
               if(has_err_pipe!=-1)
               dprintf(stdo_fd,"(%d->%d)has error pipe %d\n",getpid(),delay,has_err_pipe );
#endif

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
                  *nextCmd=getCmd(c->argv[i]," \t");
                  if(nextCmd->argc==0)
                     continue;
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
                  if(has_err_pipe>0){
                        has_err_pipe=-1;
                        checkPipe(prevCmd);
                        erro_fd=prevCmd->pipe[1];
#ifdef debug
                        dprintf(stdo_fd,"(%d->%d)error pipe %d\n",getpid(),delay,erro_fd );
#endif
                  }
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
#ifdef debug
                  dprintf(stdo_fd,"(%d->%d)+%d\n",getpid(),delay,dstqi );
#endif
                  continue;
               }



               nextCmd=c->first;  
               int child_pid;
               while(nextCmd){
                  child_pid=spawn(nextCmd);
                  if(child_pid==0) //child exec error
                  {
                     return 0;
                  }else if(child_pid<0){//fork error
                     printf("Shell Error may fork too much.\n");
                     break;
                  }
                  nextCmd=nextCmd->next;
               }

#ifdef debug
               dprintf(stdo_fd," \n");
#endif
               int status;
               nextCmd=c->first;  
               while(nextCmd){
                  close(nextCmd->pipe[0]);
                  close(nextCmd->pipe[1]);
                  //f(!kill(nextCmd->p_id,0)){
                  //
                  do{

                     if( waitpid(nextCmd->p_id, &status, WUNTRACED | WCONTINUED)==-1)
                        break;
                     
#ifdef debug
                     dprintf(stdo_fd,"%d wait close status\n",nextCmd->p_id);
#endif
                  }while(!WIFEXITED(status) && !WIFSIGNALED(status));
                  if(nextCmd->pipe[1]==erro_fd){
                     erro_fd=2;
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
            close(mysocket);
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


