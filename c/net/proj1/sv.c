#define PIPE_BUF 4096
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>   
#include  <fcntl.h>
#define queMAX 32767
#include <string.h>
#include "Array.h"
#include <limits.h>
#define debug

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
 
typedef struct PipeFD{
   int out_fd;
   int in_fd;
}PipeFD;

PipeFD newPipeFD(){
   int fd[2];
   pipe(fd);
   PipeFD pfd={fd[0],fd[1]};
   return pfd;
}
void closePipe(PipeFD *fd){
   close(fd->in_fd);
   close(fd->out_fd);
   fd->in_fd=-1;
   fd->out_fd=-1;
}
void closeOutput(PipeFD *fd){
   close(fd->out_fd);
}
void closeInput(PipeFD *fd){
   close(fd->in_fd);
}
int isNull(PipeFD *fd)
{
   return (fd->in_fd==-1 &&fd->out_fd==-1);
}
typedef struct Cmd{
   int p_id;
   int argc;
   char **argv;
   int readFd;
   int writeFd;
   PipeFD outPipe;
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
  c.parent=NULL;
  c.prev=NULL;
  c.next=NULL;
  c.outPipe.in_fd=-1;
  c.outPipe.out_fd=-1;
  c.writeFd=-1;
  c.readFd=-1;
  free(arr);
  return c;
}


int spawn(Cmd *cmd)
{
   char *prog=cmd->argv[0];
   char **arg_list=cmd->argv;

   pid_t child=0;

   //dup2Cmd(cmd);
   setCmd(cmd);
   child = fork();

   if (child != 0) {
      if(child>0){
         cmd->p_id=child;
      }
      return child;
   } else {

#ifdef debug
   dprintf(stdo_fd,"(%d)%s ",getpid(),cmd->argv[0]);
   if(!isNull(&cmd->outPipe))
      dprintf(stdo_fd,"i(%d) o(%d) ",cmd->outPipe.in_fd,cmd->outPipe.out_fd);
   dprintf(stdo_fd,"%d-%d %d-%d\n",cmd->readFd,0,cmd->writeFd,1);
#endif
      close(0);
      close(1);
      dup(cmd->readFd);
      dup(cmd->writeFd);
      if(erro_fd==-1){
         close(2);
         dup(cmd->parent->outPipe.in_fd);
      }
      else
      {
         close(2);
         dup(erro_fd);
      }
      
      int i=0;
      for(i=1024;i>2;i--){
         close(i);
      }
      
      
   
     

         execv(strcat(binPATH,prog), arg_list);
         fprintf(stderr,"Unknown command: [%s].\n",prog);
         close(0);
         close(1);
         close(2);
         exit(0);
     }
   return child;
}

void showCmd(Cmd *cmd){
   int i;
   for(i=0;i<cmd->argc;i++){
      printf("argv[%d]=%s\n",i,cmd->argv[i]);
   }

}




int setCmd(Cmd *cmd){
   int i,fd=1;


   for (i=1;i+1<cmd->argc;i++){
      if(strncmp(cmd->argv[i],">",1)==0)
      {
         char * fpath=cmd->argv[i+1];
         if(strncmp(cmd->argv[i],">>",2)==0)
         fd=open(fpath,O_WRONLY|O_CREAT|O_APPEND,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
         else
         fd=open(fpath,O_WRONLY|O_CREAT|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
         cmd->argv[i]=NULL;
         cmd->writeFd=fd;
      }
   }
   if(cmd->readFd==-1){
      if(isNull(&cmd->outPipe)){
         cmd->readFd=cmd->parent->outPipe.out_fd;

   }
   else{
      cmd->readFd=cmd->outPipe.out_fd;
   }
}
if(cmd->writeFd==-1)
{
if(cmd->next==NULL)
   cmd->writeFd=cmd->parent->outPipe.in_fd;
else{
   if(isNull(&cmd->next->outPipe))
   cmd->next->outPipe=newPipeFD();
   cmd->writeFd=cmd->next->outPipe.in_fd;
}
}

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

int main(int argc,char *argv[],char *envp[]){
   printf("Pipe buffer size=%d\n",PIPE_BUF);
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
            erro_fd=-1;
            close(mysocket);
            close(stdi_fd);
            char *remote_ip=inet_ntoa(dest.sin_addr);
            int remote_port=dest.sin_port;
            char msg[255];
            char buffer[255];

            PipeFD queFd[queMAX];
            memset(&queFd,-1,queMAX*sizeof(PipeFD));
            int qi=0;

            dprintf(stdo_fd,"(%d)%s open\n",getpid(),remote_ip);
            
            printf("****************************************\n");
            printf("** Welcome to the information server. **\n");
            printf("****************************************\n");
            printf("Your From %s:%d\n",remote_ip,remote_port);
            fflush(stdout);
            
            PipeFD console;
            console.out_fd=dup(consocket);
            console.in_fd=dup(consocket);
            Cmd *c=(Cmd*)malloc(sizeof(Cmd));
            c->p_id=getpid();
            c->outPipe=console;
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
               Cmd *subCmd=malloc(sizeof(Cmd)*c->argc);
               Cmd *nextCmd=subCmd;
               Cmd *prevCmd=NULL;
               c->first=NULL;
               for(i=0;i<c->argc;i++){
                  *nextCmd=getCmd(c->argv[i]," \t");
                  if(nextCmd->argc==0)
                     continue;
                  if(c->first==NULL){
                     c->first=nextCmd;
                     c->next=nextCmd;
                     if(!isNull(&queFd[qi])){
                        nextCmd->outPipe=queFd[qi];
                        nextCmd->readFd=queFd[qi].out_fd;

                        if(erro_fd==queFd[qi].in_fd)
                           erro_fd=-1;
                  }
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
                  if(isNull(&queFd[dstqi]))
                     queFd[dstqi]=newPipeFD();
                  prevCmd->writeFd=queFd[dstqi].in_fd;
                  if(has_err_pipe>0)
                     erro_fd=queFd[dstqi].in_fd;
                  
#ifdef debug
                  dprintf(stdo_fd,"(%d->%d)+%d",getpid(),delay,dstqi );
                  dprintf(stdo_fd,"i(%d) o(%d)\n",queFd[dstqi].in_fd,queFd[dstqi].out_fd );
#endif
               }



               int status;
               int child_pid;
               nextCmd=c->first;  
               while(nextCmd){
                  if(nextCmd->outPipe.in_fd!=console.in_fd)
                     closeInput(&nextCmd->outPipe);
                  child_pid=spawn(nextCmd);

                  if(child_pid==0) //child exec error
                  {
                     exit(0);
                  }else if(child_pid<0){//fork error
                     printf("Shell Error may fork too much.\n");
                     break;
                     //continue;
                  }

                  do{

                     if( waitpid(nextCmd->p_id, &status, WUNTRACED | WCONTINUED)==-1)
                        break;
                     
#ifdef debug
                     dprintf(stdo_fd,"%d wait close status\n",nextCmd->p_id);
#endif
                  }while(!WIFEXITED(status) && !WIFSIGNALED(status));

                  if(nextCmd->outPipe.out_fd!=console.out_fd)
                     closeOutput(&nextCmd->outPipe);
                  nextCmd=nextCmd->next;
               }
               free(subCmd);
               closePipe(&queFd[qi]);
               qi++;
               qi%=queMAX;

            }
          
            closePipe(&c->outPipe);
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


