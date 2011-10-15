#include <stdio.h>
#include <stdlib.h>
int fork_hello(){
   int node_id=0;
   node_id = fork();
   int i;
   if (node_id == 0) {
      for (i=0;i<5;i++){//root
         printf("%d pid=%d node_id=%d\n",i,getpid(),node_id);
         sleep(1);
      } 
   }else{ 
      for (i=0;i<10;i++){//node
         printf("%d pid=%d node_id=%d\n",i,getpid(),node_id);
         usleep(500000);
      }
   }

}

int main(int argc,char *argv[],char *envp[])
{
   fork_hello();
   return 0;
}
