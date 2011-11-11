#include <stdio.h>
#include <stdlib.h>
int fork_hello(int s){
   int node_id=0;
   node_id = fork();
   int i;
   if (node_id != 0) {//root
      return node_id;
   }else{ 
      for (i=0;i<5;i++){//node
         printf("%d pid=%d node_id=%d\n",i,getpid(),node_id);
         usleep(s);
      }
   }

   return 0;
}

int main(int argc,char *argv[],char *envp[])
{
   int id=0;
   id=fork_hello(100000);
   if(id==0){
      printf("pid(%d) end\n",getpid());
      return 0;
   }
   printf("pid(%d) start\n");
   id=fork_hello(200000);
   if(id==0){
      printf("pid(%d) end\n",getpid());
      return 0;
   }
   wait();
   printf("main end\n");
   return 0;
}
