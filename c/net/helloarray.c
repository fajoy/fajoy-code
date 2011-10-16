#include <stdio.h>
#include <stdlib.h>
#include "Array.h"
int main(int argc,char *argv[],char *envp[])
{
   int a=0;
   int b=1;
   printf("a=%d b=%d\n",a,b);
   printf("push &a=%p &b=%p\n",&a,&b);
   push(stack,&a);
   push(stack,&b);
   int *arr;
   int arr_size=ToArray(stack,&arr);
   printf("arrsize=%d\n",arr_size);
   printf("stack[0]=%p [1]=%p\n",*STACK.address,*(STACK.address+1));
   printf("pop=%d\n",pop(stack,int));
   printf("pop=%d\n",pop(stack,int));

   printf("a=%d b=%d\n",a,b);
   int i;
   for(i=0;i<arr_size;i++){
      int v=GetItem(arr,i,int);
      printf("arr[%d]=%d \n",i,v);
   }
   free(arr);
   
   char aaa[10]="aaaa";
   char bbb[10]="bbaa";
   char ccc[10]="cccc";
   push(stack,aaa);
   push(stack,bbb);
   push(stack,ccc);

   char **ddd;
    arr_size=ToArray(stack,&ddd);
    for(i=0;i<arr_size;i++){
      printf("ddd[%d]=%s \n",i,ddd[i]);
    }
   
   return 0;
}
