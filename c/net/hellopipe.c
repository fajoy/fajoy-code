#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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
   int tmp[2];
   char cmd[255];
   pipe(tmp);
   int tmp_out=tmp[0];
   int tmp_in=tmp[1]

   do{
      printf("> ");
      fgets(cmd,255,stdin);
      if(strcmp(cmd,":q\n")==0){
         close(tmp_in);
         printf("tmp:\n");
         break;
      }
      write(tmp_in,cmd,strlen(cmd));
   }while(1);
   do{
      int len=read(tmp_out,cmd,255);
      if(len==0){//EOF
         break;
      }else{
         if (len!=255)
            cmd[len]='\0';
         printf("%s",cmd);
      }
   }while(1);
   close(tmp_out);
   return 0;
}


