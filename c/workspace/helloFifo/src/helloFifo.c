#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>


int main(int argc,char *argv[],char *envp[]){
   char cmd[255];
   char fifoPath[255];
   sprintf(fifoPath,"/tmp/%s","fifo1");
   int status=mkfifo(fifoPath, 0666);
   printf("mkfifo %s =%d\n",fifoPath,status);
   int tmp_in=open(fifoPath,O_RDONLY|O_NONBLOCK);
   int tmp_out = open(fifoPath,O_WRONLY);
   printf("open input=%d\n",tmp_in);
   printf("open output=%d\n",tmp_out);
   printf("> ");
   while(fgets(cmd,255,stdin)!=NULL){
      if(strcmp(cmd,":q\n")==0){
         break;
      }
      write(tmp_out,cmd,strlen(cmd));
      printf("> ");
   }
   close(tmp_out);


   int tmp_in2=open(fifoPath,O_RDONLY|O_NONBLOCK);

   printf("\ntmp:");
   do{
      int len=read(tmp_in,cmd,255);
      if(len <= 0){//EOF
    	  printf("len=%d",len);
         break;
      }else{
         if (len!=255)
            cmd[len]='\0';
         printf("%s",cmd);
      }
   }while(1);
   close(tmp_in);
   close(tmp_in2);
   return 0;
}

