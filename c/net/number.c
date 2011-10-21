#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc,char *argv[],char *envp[]){
   char line[4096];
   int i=0;
   while(fgets(line,4096 ,stdin)!= NULL){
         printf("%i: %s",i++,line);
         memset(line,0,4096);
   }
   fflush(stdout);
   return 0;
}


