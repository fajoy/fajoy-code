#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include  <fcntl.h>
int main(int argc,char *argv[],char *envp[]){
   char line[4096];
   int i=0;
   if (argc>1){
      close(0);
      open(argv[1],O_RDONLY,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP);
   }
      
   while(fgets(line,4096 ,stdin)!= NULL){
         printf("%i: %s",i++,line);
         memset(line,0,4096);
   }
   fflush(stdout);
   return 0;
}


